/**
 * :tabSize=3:indentSize=3:folding=indent:
 * $Id$
 * \author Ken Keenan <mailto:ken@kaia.ie>
 * \file clac.c
 * A command-line calculator.  clac evaluates the arithmetic expression on 
 * its command line and copies the result to the Clipboard.  The expression 
 * parser is based on the article, "Let's Build a Compiler" by Jack Crenshaw, 
 * PhD (http://compilers.iecc.com/crenshaw/)
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <winuser.h>

#define ANSWER_SIZE 128L

char   *cmdLine = NULL;
char   currChar;

char   isAddOp(char ch);
void   getNextChar(void);
void   matchChar(char ch);
double getNum(void);
double getExp(void);
double getTerm(void);
double getFactor(void);
BOOL   bSuccess;
   

int main(int argc, char *argv[]){
   size_t  cmdLen = 0;
   int     ii, jj;
   double  theAnswer;
   HGLOBAL hAnswer;
   char    *pAnswer = NULL;
   
   /* Concatenate the command line into one big string */
   for(ii = 0; ii < 2; ++ii){
      for(jj = 1; jj < argc; ++jj){
         if(ii == 0)
            cmdLen += strlen(argv[jj]);
         else{
            if(jj == 1)
               strcpy(cmdLine, argv[jj]);
            else
               strcat(cmdLine, argv[jj]);
         }
      }
      if(ii == 0){
         cmdLine = (char *)malloc((cmdLen + 1) * sizeof(char));
         if(cmdLine == NULL){
            fprintf(stderr, "ERROR: Unable to allocate space for string.\n");
            exit(EXIT_FAILURE);
         }
      }
   }
   
   /* Load first character into lookahead */
   getNextChar();
   theAnswer = getExp();
   
   /* Create buffer for answer */
   hAnswer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_DDESHARE, ANSWER_SIZE);
   if(hAnswer == 0){
      fprintf(stderr, "Error: GlobalAlloc() failed\n");
      exit(EXIT_FAILURE);
   }
   /* Lock the memory block allocated to the buffer so we can write to it */
   pAnswer = (char *)GlobalLock(hAnswer);
   if(pAnswer == NULL){
      fprintf(stderr, "Error: GlobalLock() failed\n");
      exit(EXIT_FAILURE);
   }
   _snprintf(pAnswer, ANSWER_SIZE, "%lf", theAnswer);
   /* Trim trailing zeroes */
   if(strcmp(pAnswer, "0") != 0 && strchr(pAnswer, '.') != NULL){
      ii = strlen(pAnswer) - 1;
      while(pAnswer[ii] == '0'){
         pAnswer[ii] = '\0';
         ii--;
      }
      /* Trim decimal point if superfluous */
      if(pAnswer[strlen(pAnswer) - 1] == '.'){
         pAnswer[strlen(pAnswer) - 1] = '\0';
      }
   }
   /* Unlock the buffer */
   GlobalUnlock(hAnswer);
   
   /* Transfer ownership of buffer to clipboard */
   if((bSuccess = OpenClipboard(0L))){
      if((bSuccess = EmptyClipboard())){
         if(SetClipboardData(CF_TEXT, hAnswer) != 0){
            bSuccess = CloseClipboard();
         }
         else{
            bSuccess = 0;
         }
      }
   }
   
   /* Clean up and exit */
   if(cmdLine != NULL) free(cmdLine);
   if(bSuccess)
      return EXIT_SUCCESS;
   else
      return EXIT_FAILURE;
}


char isAddOp(char ch){
   return ((ch == '+') || (ch == '-'));
}


void getNextChar(void){
   static size_t currIdx = 0;
   
   if(currIdx < strlen(cmdLine))
      currChar = cmdLine[currIdx++];
   else
      currChar = '\0';
}


void matchChar(char ch){
   if(ch == currChar)
      getNextChar();
   else{
      fprintf(stderr, "Expected: %c\n", ch);
      exit(EXIT_FAILURE);
   }
}


double getNum(void){
   double val  = 0;
   char   *buf = NULL;
   size_t i    = 0;
   
   if(!isdigit(currChar)){
      fprintf(stderr, "Expected: number. Got %c\n", currChar);
      exit(EXIT_FAILURE);
   }
   buf = (char *)malloc((strlen(cmdLine) + 1) * sizeof(char));
   if(buf == NULL){
      fprintf(stderr, "Error: Out of memory!\n");
      exit(EXIT_FAILURE);
   }
   while(currChar != '\0' && (currChar == '.' || isdigit(currChar))){
      buf[i] = currChar; buf[i + 1] = '\0';
      getNextChar();
      i++;
   }
   sscanf(buf, "%lf", &val);
   free(buf);
   return val;
}


double getExp(void){
   double val;
   
   if(isAddOp(currChar))
      val = 0;
   else
      val = getTerm();
   while(isAddOp(currChar)){
      switch(currChar){
         case '+':
            matchChar('+');
            val += getTerm();
            break;
         case '-':
            matchChar('-');
            val -= getTerm();
            break;
      }
   }
   return val;
}


double getTerm(void){
   double val;
   
   val = getFactor();
   while(currChar == '*' || currChar == '/'){
      switch(currChar){
         case '*':
            matchChar('*');
            val *= getFactor();
            break;
         case '/':
            matchChar('/');
            val /= getFactor();
            break;
      }
   }
   return val;
}


double getFactor(void){
   double val;
   
   if(currChar == '('){
      matchChar('(');
      val = getExp();
      matchChar(')');
   }
   else{
      val = getNum();
   }
   return val;
}
