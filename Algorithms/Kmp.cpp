/**********************************************
 * 描述:            KMP算法实现
 * 作者:            Hurricanezwf
 * 时间:            2014.6.16  20:40
 * 参考链接:        http://kb.cnblogs.com/page/176818/
 **********************************************/
#include <iostream>
#include <cstdio>
#include <string.h>
using namespace std;

/*functions declare*/
void getT(string str, int *T);
void print(int *T, int size);

int main()
{
  string mode_string;
  cin>>mode_string;
  
  int string_size = mode_string.length();
  int *T = new int[string_size];
  int i = 0;

  //init T[]
  for(i=0; i<string_size; i++){
    T[i] = i;
  }
  
  getT(mode_string, T);                 //set array------T
  print(T, string_size);

  char buff[2000]="\0";
  FILE *pFile;
  pFile = fopen("content.txt","r");
  if(pFile == NULL){
    perror("Error opening file!");
  }
  else{
    while(fgets(buff, 2000, pFile) != NULL){
        cout<<buff<<endl;
    }
    fclose(pFile);
  }

  int count = 0;                            //记录总匹配数
  int length = strlen(buff);
  int j = 0;                                //j:[0, string_size)
  i = 0;

  while(i<length){
      if(buff[i]!=mode_string[j]){          //不匹配
          if(0 == j){
              i++;                      //已匹配数为0
          }else{
              j = T[j];                     //已匹配数不为0
          }
      }else{                                 //有匹配
          i++;
          j++;
          if(j ==  string_size){                 //完全匹配了
              j = 0;
              count++;
          }
      }
  }
  cout<<"共有"<<count<<"个词匹配"<<endl;
  delete[] T;
  return 0;
}

/*****************************************
 * Description:     get array of T
 * Parameters:      str:    mode string
 *                  T:      array       
 *****************************************/
void getT(string str, int *T)
{
  T[0] = 0;
  T[1] = 0;

  int index = 0;
  int wnd = 0;

  for(index=2; index<str.length(); index++){
    while( (wnd>0) && (str[wnd]!=str[index-1]) ){
      wnd = T[wnd];
    }
    if(str[wnd]==str[index-1]){
      wnd++;
    }
    T[index] = wnd;
  }
}

/****************************************
 * Descrpiton:  print array----T
 * Parameters:  *T:     pointer to array
 *              size:   size of the array
 ***************************************/
void print(int *T, int size)
{
  for(int i=0; i<size; i++){
    cout<<T[i]<<"  ";
  }
  cout<<endl;
}
