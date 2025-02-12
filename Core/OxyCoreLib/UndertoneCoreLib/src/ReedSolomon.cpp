#include "ReedSolomon.h"
#include <math.h>
#include <stdio.h>
#include <string.h>


using namespace OXY;

ReedSolomon::ReedSolomon(void)
{
  //TODO: calculate mm from numFreqs var
  mm = 5;           //RS code over GF(2**5) - change to suit (32 freqs)
  //mm = 4;           /* RS code over GF(2**4) - change to suit */
  
  //31
  nn = (int)(pow(2.0,mm)-1);          // nn=2**mm -1   length of codeword
  //nn = 20;          /* nn=2**mm -1   length of codeword */  

  tt = 4;           // number of errors that can be corrected
  //tt = 3;           /* number of errors that can be corrected */
  
  //23
  kk = (int)(nn-2*tt);           // kk = nn-2*tt
  //kk = 9;           /* kk = nn-2*tt */
  msg_len = 12;  // Voctro: we are using Shortened RS Code, which means we are encoding/decoding less symbols.
                 // Voctro: for Shortened-RS, we will should zero-pad from position 12 to 23 in the message array.

  pp = new int[mm+1];
  //pp={ 1, 1, 0, 0, 1} ; /* specify irreducible polynomial coeffts */
  //f(x) = 1 + x^2 + x^5
  pp[0]=1; 
  pp[1]=0;
  pp[2]=1;
  pp[3]=0;
  pp[4]=0;
  pp[5]=1;
  
  //For encode
  alpha_to = new int[nn+1];
  index_of = new int[nn+1];
  gg = new int[nn-kk+1];

  recd = new int[nn];
  data = new int[kk];
  bb = new int[nn-kk];

  //For decode
  elp = new int*[nn-kk+2];
  for (int i=0;i<(nn-kk+2);i++)
    elp[i] = new int[nn-kk];
  d = new int[nn-kk+2];
  l = new int[nn-kk+2];
  u_lu = new int[nn-kk+2];
  s = new int[nn-kk+1];

  root = new int[tt];
  loc = new int[tt];
  z = new int[tt+1];
  err = new int[nn];
  reg = new int[tt+1];

  GenerateGaloisField();

/*  printf("Look-up tables for GF(2**%2d)\n",mm);
  printf("  i   alpha_to[i]  index_of[i]\n");
  for (int i=0; i<=nn; i++)
   printf("%3d      %3d          %3d\n",i,alpha_to[i],index_of[i]);
  printf("\n\n"); */


  GeneratePoly();

}

ReedSolomon::~ReedSolomon(void)
{
  //delete all news in constructor!!!

  //For encode
  delete [] pp;
  
  delete [] alpha_to;
  delete [] index_of;
  delete [] gg;

  delete [] recd;
  delete [] data;
  delete [] bb;

  //For decode
  //delete all this!!
  
  for (int i=0;i<(nn-kk+2);i++)
    delete [] elp[i];
  delete [] elp;

  delete [] d;
  delete [] l;
  delete [] u_lu;
  delete [] s;

  delete [] root;
  delete [] loc;
  delete [] z;
  delete [] err;
  delete [] reg;
}

/* generate GF(2**mm) from the irreducible polynomial p(X) in pp[0]..pp[mm]
   lookup tables:  index->polynomial form   alpha_to[] contains j=alpha**i;
                   polynomial form -> index form  index_of[j=alpha**i] = i
   alpha=2 is the primitive element of GF(2**mm) */
void ReedSolomon::GenerateGaloisField()
{
  register int i, mask;

  mask = 1 ;
  alpha_to[mm] = 0 ;
  for (i=0; i<mm; i++)
  {
    alpha_to[i] = mask ;
    index_of[alpha_to[i]] = i ;
    if (pp[i]!=0)
      alpha_to[mm] ^= mask ;
    mask <<= 1 ;
  }
  index_of[alpha_to[mm]] = mm ;
  mask >>= 1 ;
  for (i=mm+1; i<nn; i++)
  {
    if (alpha_to[i-1] >= mask)
      alpha_to[i] = alpha_to[mm] ^ ((alpha_to[i-1]^mask)<<1) ;
    else alpha_to[i] = alpha_to[i-1]<<1 ;
      index_of[alpha_to[i]] = i ;
  }
  index_of[0] = -1 ;
}

/* Obtain the generator polynomial of the tt-error correcting, length
  nn=(2**mm -1) Reed Solomon code  from the product of (X+alpha**i), i=1..2*tt */
void ReedSolomon::GeneratePoly()
{
  register int i,j ;

  gg[0] = 2 ;    /* primitive element alpha = 2  for GF(2**mm)  */
  gg[1] = 1 ;    /* g(x) = (X+alpha) initially */
  for (i=2; i<=nn-kk; i++)
  { 
    gg[i] = 1 ;
    for (j=i-1; j>0; j--)
      if (gg[j] != 0)  gg[j] = gg[j-1]^ alpha_to[(index_of[gg[j]]+i)%nn] ;
      else gg[j] = gg[j-1] ;
    gg[0] = alpha_to[(index_of[gg[0]]+i)%nn] ;     /* gg[0] can never be zero */
  }
  /* convert gg[] to index form for quicker encoding */
  for (i=0; i<=nn-kk; i++)  gg[i] = index_of[gg[i]] ;
}

/* take the string of symbols in data[i], i=0..(k-1) and encode systematically
   to produce 2*tt parity symbols in bb[0]..bb[2*tt-1]
   data[] is input and bb[] is output in polynomial form.
   Encoding is done by using a feedback shift register with appropriate
   connections specified by the elements of gg[], which was generated above.
   Codeword is   c(X) = data(X)*X**(nn-kk)+ b(X)          */
void ReedSolomon::Encode()
{
  register int i,j;
  int feedback;

  for (i=0; i<nn-kk; i++)
    bb[i] = 0;

  for (i=kk-1; i>=0; i--)
  { 
    feedback = index_of[data[i]^bb[nn-kk-1]];
    if (feedback != -1)
    { 
      for (j=nn-kk-1; j>0; j--)
        if (gg[j] != -1)
          bb[j] = bb[j-1]^alpha_to[(gg[j]+feedback)%nn] ;
        else
          bb[j] = bb[j-1] ;
      bb[0] = alpha_to[(gg[0]+feedback)%nn] ;
    }
    else
    { 
      for (j=nn-kk-1; j>0; j--)
        bb[j] = bb[j-1];
      bb[0] = 0 ;
    }
  }
}

/* assume we have received bits grouped into mm-bit symbols in recd[i],
   i=0..(nn-1),  and recd[i] is index form (ie as powers of alpha).
   We first compute the 2*tt syndromes by substituting alpha**i into rec(X) and
   evaluating, storing the syndromes in s[i], i=1..2tt (leave s[0] zero) .
   Then we use the Berlekamp iteration to find the error location polynomial
   elp[i].   If the degree of the elp is >tt, we cannot correct all the errors
   and hence just put out the information symbols uncorrected. If the degree of
   elp is <=tt, we substitute alpha**i , i=1..n into the elp to get the roots,
   hence the inverse roots, the error location numbers. If the number of errors
   located does not equal the degree of the elp, we have more than tt errors
   and cannot correct them.  Otherwise, we then solve for the error value at
   the error location and correct the error.  The procedure is that found in
   Lin and Costello. For the cases where the number of errors is known to be too
   large to correct, the information symbols as received are output (the
   advantage of systematic encoding is that hopefully some of the information
   symbols will be okay and that if we are in luck, the errors are in the
   parity part of the transmitted codeword).  Of course, these insoluble cases
   can be returned as error flags to the calling routine if desired.   */
void ReedSolomon::Decode()
{
  register int i,j,u,q;
  int count=0, syn_error=0;

  //int elp[nn-kk+2][nn-kk], d[nn-kk+2], l[nn-kk+2], u_lu[nn-kk+2], s[nn-kk+1] ;
  //int root[tt], loc[tt], z[tt+1], err[nn], reg[tt+1] ;

  /* first form the syndromes */
  for (i=1; i<=nn-kk; i++)
  { s[i] = 0 ;
    for (j=0; j<nn; j++)
      if (recd[j]!=-1)
        s[i] ^= alpha_to[(recd[j]+i*j)%nn] ;      /* recd[j] in index form */
  /* convert syndrome from polynomial form to index form  */
    if (s[i]!=0)  syn_error=1 ;        /* set flag if non-zero syndrome => error */
    s[i] = index_of[s[i]] ;
  } ;

  if (syn_error)       /* if errors, try and correct */
  {
  /* compute the error location polynomial via the Berlekamp iterative algorithm,
  following the terminology of Lin and Costello :   d[u] is the 'mu'th
  discrepancy, where u='mu'+1 and 'mu' (the Greek letter!) is the step number
  ranging from -1 to 2*tt (see L&C),  l[u] is the
  degree of the elp at that step, and u_l[u] is the difference between the
  step number and the degree of the elp.
  */
  /* initialise table entries */
    d[0] = 0 ;           /* index form */
    d[1] = s[1] ;        /* index form */
    elp[0][0] = 0 ;      /* index form */
    elp[1][0] = 1 ;      /* polynomial form */
    for (i=1; i<nn-kk; i++)
      { elp[0][i] = -1 ;   /* index form */
        elp[1][i] = 0 ;   /* polynomial form */
      }
    l[0] = 0 ;
    l[1] = 0 ;
    u_lu[0] = -1 ;
    u_lu[1] = 0 ;
    u = 0 ;

    do
    {
      u++ ;
      if (d[u]==-1)
        { l[u+1] = l[u] ;
          for (i=0; i<=l[u]; i++)
           {  elp[u+1][i] = elp[u][i] ;
              elp[u][i] = index_of[elp[u][i]] ;
           }
        }
      else
  /* search for words with greatest u_lu[q] for which d[q]!=0 */
        { q = u-1 ;
          while ((d[q]==-1) && (q>0)) q-- ;
  /* have found first non-zero d[q]  */
          if (q>0)
           { j=q ;
             do
             { j-- ;
               if ((d[j]!=-1) && (u_lu[q]<u_lu[j]))
                 q = j ;
             }while (j>0) ;
           } ;

  /* have now found q such that d[u]!=0 and u_lu[q] is maximum */
  /* store degree of new elp polynomial */
          if (l[u]>l[q]+u-q)  l[u+1] = l[u] ;
          else  l[u+1] = l[q]+u-q ;

  /* form new elp(x) */
          for (i=0; i<nn-kk; i++)    elp[u+1][i] = 0 ;
          for (i=0; i<=l[q]; i++)
            if (elp[q][i]!=-1)
              elp[u+1][i+u-q] = alpha_to[(d[u]+nn-d[q]+elp[q][i])%nn] ;
          for (i=0; i<=l[u]; i++)
            { elp[u+1][i] ^= elp[u][i] ;
              elp[u][i] = index_of[elp[u][i]] ;  /*convert old elp value to index*/
            }
        }
      u_lu[u+1] = u-l[u+1] ;

  /* form (u+1)th discrepancy */
      if (u<nn-kk)    /* no discrepancy computed on last iteration */
        {
          if (s[u+1]!=-1)
                 d[u+1] = alpha_to[s[u+1]] ;
          else
            d[u+1] = 0 ;
          for (i=1; i<=l[u+1]; i++)
            if ((s[u+1-i]!=-1) && (elp[u+1][i]!=0))
              d[u+1] ^= alpha_to[(s[u+1-i]+index_of[elp[u+1][i]])%nn] ;
          d[u+1] = index_of[d[u+1]] ;    /* put d[u+1] into index form */
        }
    } while ((u<nn-kk) && (l[u+1]<=tt)) ;

    u++ ;
    if (l[u]<=tt)         /* can correct error */
     {
  /* put elp into index form */
       for (i=0; i<=l[u]; i++)   elp[u][i] = index_of[elp[u][i]] ;

  /* find roots of the error location polynomial */
       for (i=1; i<=l[u]; i++)
         reg[i] = elp[u][i] ;
       count = 0 ;
       for (i=1; i<=nn; i++)
        {  q = 1 ;
           for (j=1; j<=l[u]; j++)
            if (reg[j]!=-1)
              { reg[j] = (reg[j]+j)%nn ;
                q ^= alpha_to[reg[j]] ;
              } ;
           if (!q)        /* store root and error location number indices */
            { root[count] = i;
              loc[count] = nn-i ;
              count++ ;
            };
        } ;
       if (count==l[u])    /* no. roots = degree of elp hence <= tt errors */
        {
  /* form polynomial z(x) */
         for (i=1; i<=l[u]; i++)        /* Z[0] = 1 always - do not need */
          { if ((s[i]!=-1) && (elp[u][i]!=-1))
               z[i] = alpha_to[s[i]] ^ alpha_to[elp[u][i]] ;
            else if ((s[i]!=-1) && (elp[u][i]==-1))
                    z[i] = alpha_to[s[i]] ;
                 else if ((s[i]==-1) && (elp[u][i]!=-1))
                        z[i] = alpha_to[elp[u][i]] ;
                      else
                        z[i] = 0 ;
            for (j=1; j<i; j++)
              if ((s[j]!=-1) && (elp[u][i-j]!=-1))
                 z[i] ^= alpha_to[(elp[u][i-j] + s[j])%nn] ;
            z[i] = index_of[z[i]] ;         /* put into index form */
          } ;

  /* evaluate errors at locations given by error location numbers loc[i] */
         for (i=0; i<nn; i++)
           { err[i] = 0 ;
             if (recd[i]!=-1)        /* convert recd[] to polynomial form */
               recd[i] = alpha_to[recd[i]] ;
             else  recd[i] = 0 ;
           }
         for (i=0; i<l[u]; i++)    /* compute numerator of error term first */
          { err[loc[i]] = 1;       /* accounts for z[0] */
            for (j=1; j<=l[u]; j++)
              if (z[j]!=-1)
                err[loc[i]] ^= alpha_to[(z[j]+j*root[i])%nn] ;
            if (err[loc[i]]!=0)
             { err[loc[i]] = index_of[err[loc[i]]] ;
               q = 0 ;     /* form denominator of error term */
               for (j=0; j<l[u]; j++)
                 if (j!=i)
                   q += index_of[1^alpha_to[(loc[j]+root[i])%nn]] ;
               q = q % nn ;
               err[loc[i]] = alpha_to[(err[loc[i]]-q+nn)%nn] ;
               recd[loc[i]] ^= err[loc[i]] ;  /*recd[i] must be in polynomial form */
             }
          }
        }
       else    /* no. roots != degree of elp => >tt errors and cannot solve */
         for (i=0; i<nn; i++)        /* could return error flag if desired */
             if (recd[i]!=-1)        /* convert recd[] to polynomial form */
               recd[i] = alpha_to[recd[i]] ;
             else  recd[i] = 0 ;     /* just output received codeword as is */
     }
   else         /* elp has degree has degree >tt hence cannot solve */
     for (i=0; i<nn; i++)       /* could return error flag if desired */
        if (recd[i]!=-1)        /* convert recd[] to polynomial form */
          recd[i] = alpha_to[recd[i]] ;
        else  recd[i] = 0 ;     /* just output received codeword as is */
  }
  else       /* no non-zero syndromes => no errors: output received codeword */
  for (i=0; i<nn; i++)
     if (recd[i]!=-1)        /* convert recd[] to polynomial form */
       recd[i] = alpha_to[recd[i]] ;
     else  recd[i] = 0 ;
}

// Voctro API functions 
// encoding 
void ReedSolomon::SetMessage(const std::vector<int> message)
{

    msg_len = message.size(); // update input message length for Shortened RS
    for (int i=0;i < msg_len;++i)
        data[i] = message[i];

    // pad with zeros for Shortened RS code.
    if (msg_len < kk)
        memset(data + msg_len,0, (kk - msg_len)*sizeof(int));
}

void ReedSolomon::GetCode(std::vector<int> &code)
{
    code.clear();
    // fill with message values
    for (int i=0;i < msg_len;++i)
        code.push_back(data[i]);
    // fill with correction code stored in bb[0:7]
    for (int i=0;i <  (nn-kk);++i)
        code.push_back(bb[i]);
    // output vector should have a legnth of 20 (12 message + 8 correction code)
}

// decoding received code
void ReedSolomon::SetCode(const std::vector<int> code)
{
    if ((code.size() < kk) && (msg_len < kk))
    {
         // copy received error code digits (8)
        for (int i=0; i < (nn - kk); ++i)
           recd[i] = code[msg_len+i];

        // copy received message
        for (int i=0; i < msg_len; ++i)
            recd[(nn - kk)+i] = code[i];
        //For Shortened RC Code,  fill padded zeros to obtain the correct size        
        memset(recd + msg_len + (nn - kk),0, (kk - msg_len)*sizeof(int));

        // check if this is necessary        
        // convert received data from polynomial form to index form (as it was in the example source code)
        for (int i=0; i<nn; i++)
             recd[i] = index_of[recd[i]] ;          /* put recd[i] into index form */

    }
}

void ReedSolomon::GetMessage(std::vector<int> &message)
{
   message.clear();
    // fill with message values
    for (int i=0;i < msg_len;++i)
        message.push_back(recd[(nn - kk)+ i]);
}
