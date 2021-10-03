// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.0 -- Março de 2015

// Demonstração das funções POSIX de troca de contexto (ucontext.h).

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

// operating system check
#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este codigo foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilacao e execucao em outros ambientes e responsabilidade do usuario.
#endif

#define _XOPEN_SOURCE 600	/* para compilar no MacOS */

#define STACKSIZE 64*1024	/* tamanho de pilha das threads */

ucontext_t ContextPing, ContextPong, ContextMain ;

/*****************************************************/

void BodyPing (void * arg)
{
   int i ;

   printf ("%s: inicio\n", (char *) arg) ;

   for (i=0; i<4; i++)
   {
      printf ("%s: %d\n", (char *) arg, i) ;
      swapcontext (&ContextPing, &ContextPong) ;    // Saves the current processor's context to ContextPing and 
   }                                                // restores values of ContextPong, entering BodyPong function
   printf ("%s: fim\n", (char *) arg) ;

   swapcontext (&ContextPing, &ContextMain) ;       // Returns to main context on finish
}

/*****************************************************/

void BodyPong (void * arg)
{
   int i ;

   printf ("%s: inicio\n", (char *) arg) ;

   for (i=0; i<4; i++)
   {
      printf ("%s: %d\n", (char *) arg, i) ;
      swapcontext (&ContextPong, &ContextPing) ;    // Saves the current processor's context to ContextPong and
   }                                                // restores values of ContextPing, returning to BodyPing function
   printf ("%s: fim\n", (char *) arg) ;

   swapcontext (&ContextPong, &ContextMain) ;
}

/*****************************************************/

int main (int argc, char *argv[])
{
   char *stack ;

   printf ("main: inicio\n") ;


    /*  P1.1 - Comment
        The function getcontext() initializes the variable pointed, in this case ContextPing,
        with the current active context of the processor. So it saves the registers
        values stored in the processor inside the pointed variable.

        The sister function, setcontext(), gets the values of the pointed variable and
        restores its values in the processor. This values are not only the register,
        but also other importante informations such as the Program Counter (PC) and the
        Stack Pointer (SP).
    */
   getcontext (&ContextPing) ;  // Saves the corrent context to ContextPing

   stack = malloc (STACKSIZE) ;
   if (stack)
   {
       /* P1.2 - Comment
            - uc_stack is the stack used by this context
                - ss_sp points to the base address of the stack
                - ss_size refers to the stack's size
                - ss_flags sets flag SS_AUTODISARM that clears the alternate signal stack
                settings on entry to the signal handler. Or zero for none.

            - uc_link points to the context that will be resumed when the
            current context terminates.
       */
      ContextPing.uc_stack.ss_sp = stack ;          // Initialize ContextPing's pointer stack
      ContextPing.uc_stack.ss_size = STACKSIZE ;    // Defines its stack size
      ContextPing.uc_stack.ss_flags = 0 ;           // Disable stack flags
      ContextPing.uc_link = 0 ;                     // Tells the process to exit on function's return
   }
   else
   {
      perror ("Erro na criação da pilha: ") ;
      exit (1) ;
   }

    /* P1.1 - Comment
        This function basically modifies the context stored in the first variable.
        Makes the Program Counter (PC) saved before points to the second variable's
        PC and the Stack Pointer (SP) saved receives one parameter: the string address.

        In summary, when this context is activated it will call the function BodyPing.
    */
   makecontext (&ContextPing, (void*)(*BodyPing), 1, "    Ping") ;  // Makes the context's PC point to the function
                                                                    // BodyPing and makes the previous set Stack point
                                                                    // to the specified string

   getcontext (&ContextPong) ;  // Same as line 77

   stack = malloc (STACKSIZE) ;
   if (stack)   // Same as lines 92 to 95, but to a different context
   {
      ContextPong.uc_stack.ss_sp = stack ;
      ContextPong.uc_stack.ss_size = STACKSIZE ;
      ContextPong.uc_stack.ss_flags = 0 ;
      ContextPong.uc_link = 0 ;
   }
   else
   {
      perror ("Erro na criação da pilha: ") ;
      exit (1) ;
   }

   makecontext (&ContextPong, (void*)(*BodyPong), 1, "        Pong") ; // Same as line 110, but to a different context

    /* P1.1 - Comment
        This function firstly saves the current context in the first variable passed,
        just like the getcontext(). After that the context stored in the second variable is
        restored to the processor, similar to what happens in the setcontext().
    */
   swapcontext (&ContextMain, &ContextPing) ;   // Saves the main context and restore the ping context
                                                // to the processor's register, entering BodyPing function

   swapcontext (&ContextMain, &ContextPong) ;   // Saves main context and restores ContextPong, in this example will only
                                                // reenter the function to print its final message

   printf ("main: fim\n") ;

   exit (0) ;
}