/*Copyright (c) 2008, Christopher Lee
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL CHRISTOPHER LEE BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <iostream>
#include <fstream>
#include <string>
#include "mpi.h"
#define INT 1
#define STRING 2

bool r_(int &, int);
bool r_(std::string &, int);

//trimming lines from left
inline std::string ltrim(const std::string &source, const std::string& t=" ")
{
  std::string str=source;
  return str.erase(str.find_last_not_of(t)+1);
}
//trimming lines from right
inline std::string rtrim(const std::string &source, const std::string& t=" ")
{
  std::string str=source;
  return str.erase(0,str.find_first_not_of(t));
}
//trimming both left and right
inline std::string trim(const std::string &source, const std::string& t=" ")
{
  std::string str=source;
  return ltrim(rtrim(str, t),t);
}
//removing comment lines
inline std::string comment(const std::string &source, const std::string& t="#")
{
  std::string str=source;
  int npos = str.find_first_of(t);
  if(npos < 0) return str; else return str.erase(str.find_first_of(t), str.length());
}

//get a line, trim and remove comments.
std::string getline(std::ifstream &in)
{
  bool gotline=false;
  std::string s;
  do
    {
      getline(in,s);
      s=comment(trim(s));
      if(in.eof())
	{	  gotline=true;	                         }
      else if(in.fail()) 
	{
	  std::cerr<<" Fail bit set"<<std::endl; 
	  s="internal_fail"; 
	  gotline=true;
	}
      else if(in.bad()) 
	{	
	  std::cerr<<" Bad bit set"<<std::endl;	 
	  s="internal_bad"; 
	  gotline=true;
	}

      if(!s.empty()) //check the comment 
	{	  gotline=true;                        	 }
      
    } while (!gotline);

  return s;
}

bool sanity_check(std::string command)
{
  //is "rm"
  if(    (static_cast<int>(command.find("rm ")) >= 0)
       ||(static_cast<int>(command.find(";rm ",0))>= 0)
       ||(static_cast<int>(command.find(" rm ",0))>= 0)
       ||(static_cast<int>(command.find("-rf",0)) >= 0)
     )
    {
      std::cerr<<"rm check failed: "<<command<<":"<<std::endl;
      return false;
    }

  return true;
}

//send int
bool s_(int m, int to)
{
  MPI_Send(&m, 1, MPI_INTEGER, to, INT, MPI_COMM_WORLD);
  return true;
}

//send string
bool s_(std::string str, int to)
{
  int l=str.length();
  s_(l,to);
  r_(l,to);
  if(l > 0)
    {
      const char * s=(str.c_str());
      MPI_Send((void *)s, l+1, MPI_CHAR, to, STRING, MPI_COMM_WORLD);
      return true;
    }
  else {
    return false;
  }
    
}

//receive int
bool r_(int &m, int from)
{
  MPI_Recv(&m,1, MPI_INTEGER, from, INT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  return true;
}

//receive string
bool r_(std::string &str, int from)
{
  int l=0;
  r_(l,from);
  char *s = NULL;
  if(l < 0)
    {
      s_(-1,from);
      return false;
    }
  else
    {    
      s=new char[l+1];
      if(s==NULL)
	{
	  s_(-2,from);
	  return false;
	}
      else
	{
	  s_(l,from);
	  MPI_Recv(s,l+1,MPI_CHAR, from, STRING, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	  str = std::string(s);
	  delete []s;
	  return true;
	}
    }

}


//send exit commands (send int=-1)
void m_exit(int s, int root)
{
  int e=-1;
  int d=0;
  for (int i=1;i<s;i++)
    {r_(d,i); s_(e,d);}
}


int main(int argc, char ** argv)
{
  int rank, root, size;
  root=0;

  //initialize
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);


  if(rank == root)
    {
      //master
      int cnode=0;
      int data =1;

      //read input
      if(argc > 1)
	{
	  std::string line;
	  std::ifstream input;
	  input.open(*(argv+1), std::ios::in);
	  line="";
	  int nline=-1;
	  if(argc > 2)
	    {
	      //skip some lines
	      nline = atoi(*(argv+2));
	      if(nline >= 0)
		for (int i=0;i<nline;i++)
		  line=getline(input);
	    }
	  std::cerr<<"Skipping "<<nline<<" lines"<<std::endl;

	  int ncount=-1;
	  int count=0;
	    
	  if(argc > 3)
	    {
	    ncount = atoi(*(argv+3));
	    std::cerr<<"Counting "<<ncount<<" lines"<<std::endl;
	    }
	  
	  line=getline(input);
	  ++count; 
	  bool stop=false;
	  if (line == "internal_fail") stop=true;
	  if (line == "internal_bad")  stop=true;
	  //loop through file, get lines, send line to slave
	  while(!stop)
	    {
	      r_(cnode, MPI_ANY_SOURCE);
	      s_(cnode, cnode);
	      if(!s_(line , cnode)) stop=true;
	      line = getline(input);
	      if (line == "internal_fail") stop=true;
	      if (line == "internal_bad")  stop=true;
	      ++count;
	      if(input.eof() || (count  >= ncount && ncount > 0))  stop=true;
	    }
      input.close();
	}

      m_exit(size,0);
    }
  else
    {
      //slave
      int cnode=0;
      std::string command;

      while(cnode>=0)
	{
	  //get command
	  s_(rank , root);//send
	  r_(cnode, root);//receive

	  if(cnode > 0)
	    {
	      //command
	      command="";
	      r_(command, root);

	      if(sanity_check(command))
		{
		  std::cout<<"Rank: "<<rank<<", running : "<<command<<std::endl;
		  system(command.c_str());
		}
	    }
	  else
	    {
	      ///exiting if cnode <=0 (cnode=-1 for exit)
	    }
	}
    }

  MPI_Finalize();
  return 0;
}
