/**习惯把密码明文存在本地文件中，这个小程序可以把存的密码以密文形式保存**/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
int chartoasc(char c);
int xor(int i);
char asctochar(int a);
int rand_num();
void encrypt(const char *org_pass,char *new_pass);
void decrypt(const char *new_pass,char *org_pass);
 
    /*encrypt(password,&new_pass);
    decrypt(password,&org_pass);*/

 
/**将字符转换为ASCII值**/
int chartoasc(char c)
{
    int i= 0;
    i = c;
    return i;
}
 
/**将ASCII进行异或运算，产生新的ASCII值**/
int xor(int i)
{
    int m = 27;
    int result = 0;
    if(59==i || 100==i)
    {
        return i;
    }
    result = i^m;
    return result;
}
 
/**将ASCII值转换为字符**/
char asctochar(int a)
{
    char c;
    c = a;
    return c;
}
 
/**输入原密码产生新的密码**/
void encrypt(const char *org_pass,char *new_pass)
{
    char org_password[50];
    char new_password[50];
    int len = 0;
    int i = 0;
    int asc = 0 ;
    char ch = 0;
    int x = 0;
 
    bzero(org_password,sizeof(org_password));
    bzero(new_password,sizeof(new_password));
    strcpy(org_password, org_pass);
    len = strlen(org_password);
    for(i=0 ; i<len ; i++)
    {
        ch = org_password[i];
        asc = chartoasc(ch);
        x = xor(asc);
        new_password[i] = asctochar(x);
    }
    strcpy(new_pass,new_password);
 
    return 0;
}
 
/**输入加密后的密码返回原密码**/
void decrypt(const char *new_pass,char *org_pass)
{
    char new_password[50];
    char org_password[50];
    char ch;
    int a = -1;
    int len =0;
    int i=0;
    int x = -1;
 
    bzero(new_password,sizeof(new_password));
    bzero(org_password,sizeof(org_password));
 
    strcpy(new_password,new_pass);
    len = strlen(new_password);
    for(i=0;i<len;i++)
    {
        ch = new_password[i];
        a = chartoasc(ch);
        x = xor(a);
        org_password[i]=asctochar(x);
    }
    strcpy(org_pass,org_password);
 
    return 0;
}