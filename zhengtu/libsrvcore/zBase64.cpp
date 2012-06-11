/**
 * \brief base64±àÂë½âÂëº¯Êı
 *
 * 
 */

//#include <zebra/srvEngine.h>
#include <string>
#include <baseLib/base64Lib.h>

void base64_encrypt(const std::string &input,std::string &output)
{
  int  len = input.length() * 4 / 3 + 4;
  if(len>0)
  {
	  char* dest=new char[len];

		  len = Base64_Encode((PBYTE)input.c_str(),input.length(),dest,len,0);
		  dest[len] = 0;
	  output = dest;
	  if(dest)
	      delete []dest;
  }
}

void base64_decrypt(const std::string &input,std::string &output)
{
  int  len = input.length();
  if(len>0)
  {
	  char* dest=new char[len];

		  len = Base64_Decode((char*)input.c_str(),len,(PBYTE)dest,len);
		  dest[len] = 0;
	  output = dest;
	  if(dest)
		  delete []dest;
  }
}

