#include <string.h>
#include <stdlib.h>

/*C substring function: It returns a pointer to the substring */

char *substring(char *string, int position, int length)
{
   char *p;
   int c;
   p = malloc((length+1)*sizeof(char));
   bzero(p, (length+1)*sizeof(char)); 
   for (c = 0; c < length; c++)
	{
      *(p+c) = *(string+position-1);      
      string++;  
   } 
   *(p+c) = '\0';
   return p;
}


/**
 * Function to remove all occurrences of a character from the string.
 */
void removeAll(char * str, const char toRemove)
{
    int i, j;
    int len = strlen(str);

    for(i=0; i<len; i++)
    {
        /*
         * If the character to remove is found then shift all characters to one
         * place left and decrement the length of string by 1.
         */
        if(str[i] == toRemove)
        {
            for(j=i; j<len; j++)
            {
                str[j] = str[j+1];
            }

            len--;

            // If a character is removed then make sure i doesn't increments
            i--;
        }
    }
}