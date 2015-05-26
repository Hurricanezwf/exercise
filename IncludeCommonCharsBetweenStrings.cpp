/*=============================================================================
 *  问题描述: 给定两个字符串a和b,如果两者具有相同的字符且数量也相同,顺序可以不
 相同，满足此条件返回true，否则返回false。空串也算。
    举例：
    "abc"和"bca"-----true
    "abc"和"abd"-----false
    "aab"和"abb"-----false
    ""和""-----------true
    
 * 备注：可采用哈希表
 ==============================================================================*/

#include <iostream>
#include <map>
#include <cstring>
#include <string>
#include <cassert>
using namespace std;

bool ang(const string &a, const string &b);
void Print(std::map<const char, int> &m);

int main(int argc, char* argv[])
{
    assert( argc == 3 );

    string a(argv[1]);
    string b(argv[2]);
    cout<<"a="<<a<<endl<<"b="<<b<<endl;

    cout<<ang(a, b)<<endl;
    
    return 0;
}

bool ang(const string &a, const string &b )
{
    if( (a.size() == 0) && (b.size() == 0) ){
        return true;
    }
    else if( (a.size() == 0) || (b.size() == 0) ){
        return false;
    }

    std::map<const char, int> m;

    for(std::string::const_iterator str_it = a.begin(); str_it != a.end(); str_it++){
        std::map<char, int>::iterator it = m.find(*str_it);

        if( m.end() == it ){
            m.insert( std::pair<const char, int>(*str_it, 1) );
        }
        else{
            it->second++;
        }
    }

    for(std::string::const_iterator str_it = b.begin(); str_it != b.end(); str_it++){
        std::map<char, int>::iterator it = m.find(*str_it);

        if( m.end() == it ){
            it->second = -1;
        }
        else{
            it->second--;
        }
    }

    for(std::map<const char, int>::iterator it = m.begin(); it != m.end(); it++){
        if( 0 != it->second){
            return false;
        }
    }

    return true;
}

void Print(std::map<const char, int> &m)
{
    std::map<const char, int>::iterator it;
    for(it = m.begin(); it != m.end(); it++)
    {
        cout<<it->first<<"    "<<it->second<<endl;
    }
}
