#include <stdio.h>
#include <stdlib.h>
#include<string.h>

typedef struct {unsigned char B;
                unsigned char G;
                unsigned char R;
               }pixel;

    /* calculam latimea imaginii sursa */

   unsigned int latime_imagine  ( char * nume_fisier_sursa)
   {
       FILE * fin;
       fin=fopen(nume_fisier_sursa,"rb");
       if(fin==NULL)
       {
        return 0 ;

       }

       unsigned int latime_imagine;
       fseek(fin,18,SEEK_SET);
       fread(&latime_imagine,sizeof(unsigned int),1,fin);

       fclose(fin);

       return latime_imagine;


   }

      /*calculam inaltimea imaginii sursa */

   unsigned int inaltime_imagine  (char * nume_fisier_sursa)
   {
       FILE * fin;
       fin=fopen(nume_fisier_sursa,"rb");
       if(fin==NULL)
       {
           return 0;

       }

       unsigned int inaltime_imagine;
       fseek(fin,22,SEEK_SET);
       fread(&inaltime_imagine,sizeof(unsigned int),1,fin);

       fclose(fin);

       return inaltime_imagine;
   }


      /* salvam intr-un vector header ul imaginii sursa */

   void header_imagine_sursa ( char * nume_fisier_sursa, unsigned char **H)
   {

       FILE * fin;
       fin=fopen(nume_fisier_sursa,"rb");
       if(fin==NULL)
       {
           printf("Nu am gasit imaginea sursa din care citesc la memorarea header-ului.");
           return ;
       }


       (*H)=(unsigned char *)malloc(54*sizeof(unsigned char ));
       if((*H)==NULL)
       {
           printf("Nu s-a putut aloca memoriei pentru construirea vectorului ce salveaza header-ul");
           return ;
       }


       int i;
       for(i=0;i<54;i++)
       {
           fread(&(*H)[i],sizeof(unsigned char),1,fin);

       }

       fclose(fin);

   }


   /* liniarizam imaginea sursa */

   void liniarizare_imagine (char * nume_fisier_sursa, pixel **L, unsigned int H_imagine, int W_imagine)
   {
       FILE *fin;
       fin=fopen(nume_fisier_sursa,"rb");
       if(fin==NULL)
       {
           printf("Nu am gasit imaginea sursa din care citesc pentru liniarizarea imaginii.");
           return ;
       }

       fseek(fin,54,SEEK_SET);

       *L=(pixel *)malloc(H_imagine*W_imagine*sizeof(pixel));
       if((*L)==NULL)
       {
           printf("Nu s-a putut aloca memorie pentru vectorul de liniarizare");
           return ;
       }

       int padding;
       if(W_imagine%4!=0)
        padding=4-(3*W_imagine)%4;

       else
            padding=0;

       int i,j;
       pixel x;
       for(i=H_imagine-1;i>=0;i--)

       {
           for(j=0;j<W_imagine;j++)
           {

            /* citesc fiecare pixel din imaginea sursa pentru fiecare canal de culoare */
            fread(&x.B, 1,1,fin);
            fread(&x.G, 1,1,fin);
            fread(&x.R, 1,1,fin);
            /* alipirea liniilor din imaginea sursa se va efectua de sus in jos */
            (*L)[W_imagine*i+j]=x;

           }
        /* sarim peste padding la fiecare linie */
        fseek(fin,padding,SEEK_CUR);
       }
       fclose(fin);

   }

   /*deliniarizam imaginea cu ajutorul vectorului de liniarizare si
    cu vectorul in care am pastrat header-ul imaginii sursa*/

   void deliniarizare( char * nume_fisier_sursa, char * nume_fisier_destinatie, pixel *L ,
                      unsigned int H_imagine, unsigned int W_imagine )
   {
       FILE *fout;
       fout=fopen(nume_fisier_destinatie,"wb");
      if(fout==NULL)
     {
      printf("Eroare la deschiderea fisierului destinatie pentru deliniarizare.");
      return ;

     }
       unsigned char *H;
       header_imagine_sursa(nume_fisier_sursa,&H);

       int i,j;
       for(i=0;i<54;i++)
        fwrite(&H[i],sizeof(unsigned char),1,fout);

       int padding;
       if(W_imagine%4!=0)
        padding=4-(3*W_imagine)%4;
    else
        padding=0;

     pixel black;
     black.B=black.G=black.R=0;
     int p;

       for(i=H_imagine-1;i>=0;i--)
       {p=padding;

           for(j=0;j<W_imagine;j++)
           {


            fwrite(&L[W_imagine*i+j].B,sizeof(char),1,fout);
            fwrite(&L[W_imagine*i+j].G,sizeof(char),1,fout);
            fwrite(&L[W_imagine*i+j].R,sizeof(char),1,fout);
       }
       while(p!=0)/*afisam padding-ul sub forma unor pixeli negri in cazul in care acesta exista*/
            {fwrite(&black.B,1,1,fout);
             p--;}

   }
   fclose(fout);
   free(H);
   }

    /* operatia de sau-exclusiv dintre doi pixeli */

   pixel P1_sau_exclusiv_P2 ( pixel P1,pixel P2)
   {
       pixel P_nou;
       P_nou.B=P1.B^P2.B;
       P_nou.G=P1.G^P2.G;
       P_nou.R=P1.R^P2.R;
       return P_nou;

   }

   /* operatia de sau-exclusiv dintre un pixel si un numar intreg fara semn pe 32 de biti */

   pixel P_sau_exclusiv_X ( pixel P, unsigned int  X)
   {
       pixel P_nou;
       typedef union{ unsigned int t;
                      unsigned char b[4]; }bytes;
       bytes Y;
       Y.t=X;

       P_nou.B=P.B^Y.b[0];
       P_nou.G=P.G^Y.b[1];
       P_nou.R=P.R^Y.b[2];
       return P_nou;



   }

    /* functia XORSHIFT32 adaptata pentru o secventa de lungime 2*WxH_imagine */

   void XORSHIFT32 ( unsigned int ** R, char * nume_fisier_text, unsigned int WxH_imagine)
   {
        FILE *fin;
        fin=fopen(nume_fisier_text,"r");
        if(fin==NULL)
        {
            printf("Eroare la deschiderea fisierului text");
            return;
        }
        unsigned int R0;
        fscanf(fin,"%u",&R0);

       *R=(unsigned int *)malloc(2*WxH_imagine*sizeof(unsigned int));
       if((*R)==NULL)
       {
           printf("Nu s-a putut aloca memorie pentru vectorul R.");
           return;

       }
       unsigned int k,lungime;
       (*R)[0]=R0;/* seed */
       lungime=2*WxH_imagine;

       for(k=1;k<lungime;k++)
       {
           (*R)[k]=(*R)[k-1]^(*R)[k-1]<<13;
           (*R)[k]=(*R)[k]^(*R)[k]>>17;
           (*R)[k]=(*R)[k]^(*R)[k]<<5;
       }
       fclose(fin);

   }


   /*generarea unei permutari aleatoare */

   void Durstenfeld( unsigned int **Permutare, unsigned int *R, unsigned int WxH_imagine_R)
   {

       (*Permutare)=(unsigned int *)malloc(WxH_imagine_R*sizeof(unsigned int));
       if((*Permutare)==NULL)
       {
           printf("Nu s-a putut aloca memorie pentru vectorul Permutare.");
           return ;
       }

       unsigned int k,r,aux;
       for(k=0;k<WxH_imagine_R;k++)
        (*Permutare)[k]=k;

       for(k=WxH_imagine_R-1;k>=1;k--)
       {
          r=R[WxH_imagine_R-k]%(k+1);/* număr aleator din intervalul (0,k) */
          aux=(*Permutare)[r];
          (*Permutare)[r]=(*Permutare)[k];
          (*Permutare)[k]=aux;

       }

   }


    /*permutam pixelii din imaginea sursa cu ajutorul permutarii anterioare si obtinem o noua imagine */

    void permutare_pixeli( unsigned int *Permutare, pixel *L, pixel **Lpermutat,
                           unsigned int WxH_imagine_Permutare)
    {

       (*Lpermutat)=(pixel *)malloc(WxH_imagine_Permutare*sizeof(pixel));
       if((*Lpermutat)==NULL)
       {
           printf("Nu s-a putut aloca memorie pentru vectorul de pixeli permutati.");
           return ;
       }

       unsigned int k;
       for(k=0;k<WxH_imagine_Permutare;k++)
       {
           (*Lpermutat)[Permutare[k]]=L[k];

       }

    }
    /* imaginea cu pixelii permutati */

    void imagine_permutata(char *nume_fisier_sursa, char * nume_fisier_destinatie, unsigned int *Permutare,
                            pixel *L, pixel *Lpermutat,unsigned int W_imagine,unsigned int H_imagine, unsigned int WxH_imagine)

    {
        permutare_pixeli(Permutare,L,&Lpermutat,WxH_imagine);
        deliniarizare(nume_fisier_sursa, nume_fisier_destinatie,Lpermutat,H_imagine,W_imagine);
    }

    /* criptarea imaginii */

    void criptare (  char * nume_fisier_text, pixel *Lpermutat,unsigned int *R, pixel **C,
                    unsigned int W_imagine, unsigned int H_imagine, char *nume_fisier_sursa,
                    char * nume_imagine_criptata)
    {
       unsigned int WxH_imagine=W_imagine*H_imagine;/* calcularea inaltimii* latimii */
       FILE *fin;
        fin=fopen(nume_fisier_text,"r");
        if(fin==NULL)
        {
            printf("Eroare la deschiderea fisierului text in functia de criptare.");
            return;
        }

       unsigned int SV;

       fscanf(fin,"%u",&SV);
       fscanf(fin,"%u",&SV);


       unsigned int k;
       (*C)=(pixel*)malloc(WxH_imagine*sizeof(pixel));
        if((*C)==NULL)
        {
            printf("Nu s-a putut aloca memorie");
            return;

        }

       (*C)[0]=P_sau_exclusiv_X(P_sau_exclusiv_X(Lpermutat[0],SV),R[WxH_imagine]);
       for(k=1;k<WxH_imagine;k++)
       {
           (*C)[k]=P_sau_exclusiv_X(P1_sau_exclusiv_P2(Lpermutat[k],(*C)[k-1]),R[WxH_imagine+k]);
       }
       fclose(fin);
       deliniarizare(nume_fisier_sursa,nume_imagine_criptata,(*C),H_imagine,W_imagine);
    }


    void inversa (unsigned int **Inversa, char * nume_fisier_text, unsigned int WxH_imagine)
    {
        unsigned int *Permutare,*R;
        XORSHIFT32(&R,nume_fisier_text,WxH_imagine);
        Durstenfeld(&Permutare,R,WxH_imagine);

        (*Inversa)=(unsigned int *)malloc(WxH_imagine*sizeof(unsigned int *));
        if((*Inversa)==NULL)
        {
            printf("Nu s-a putut aloca memorie pentru Inversa.");
            return;

        }
        unsigned int i;
        for(i=1;i<WxH_imagine;i++)
            (*Inversa)[Permutare[i]]=i;
        free(R);
        free(Permutare);
    }

      void modificare_imgcriptata( pixel *C, unsigned int *R, pixel **C_prim, unsigned int WxH_imagine,
                                  char * nume_fisier_text)

   {
       FILE *fin;
        fin=fopen(nume_fisier_text,"r");
        if(fin==NULL)
        {
            printf("Eroare la deschiderea fisierului text");
            return;
        }

        unsigned int SV;
        fscanf(fin,"%u",&SV);
        fscanf(fin,"%u",&SV);


       unsigned int k;
       (*C_prim)=(pixel*)malloc(WxH_imagine* sizeof(pixel));
       if((*C_prim)==NULL)
       {
           printf("Nu s-a putut aloca memorie pentru C_prim (imaginea intermediara la decriptare).");
           return ;

       }

       (*C_prim)[0]=P_sau_exclusiv_X(P_sau_exclusiv_X(C[0],SV),R[WxH_imagine]);
       for(k=1;k<WxH_imagine;k++)
       {
           (*C_prim)[k]=P_sau_exclusiv_X(P1_sau_exclusiv_P2(C[k-1],C[k]),R[WxH_imagine+k]);
       }
       fclose(fin);

   }


   void decriptare (pixel *C, pixel **D,unsigned int *R,
                    unsigned int W_imagine,unsigned int H_imagine, char * nume_fisier_sursa,
                    char * nume_imagine_decriptata,char * nume_fisier_text)
   {   unsigned int WxH_imagine;
       WxH_imagine=W_imagine*H_imagine;

       unsigned int *Inversa;
       pixel *C_prim;
       /* apelam functia ce ne da inversa si cea ce ne modifica imaginea criptata */
       inversa(&Inversa,nume_fisier_text,WxH_imagine);
       modificare_imgcriptata(C,R,&C_prim,WxH_imagine,nume_fisier_text);

       (*D)=(pixel *)malloc(WxH_imagine * sizeof(pixel));
       if((*D)==NULL)
       {
           printf("Nu s-a putut aloca memorie pentru vectorul ce va memora imaginea decriptata");
           return ;
       }

       int k;
       for(k=0;k<WxH_imagine;k++)
       {
           (*D)[Inversa[k]]=C_prim[k];
       }
       deliniarizare(nume_fisier_sursa,nume_imagine_decriptata,(*D),H_imagine,W_imagine);
       free(Inversa);
       free(C_prim);

   }


   void test_chi_patrat(unsigned int W_imagine,unsigned int H_imagine,char * cale_imagine )
   {
       pixel *L;
       liniarizare_imagine(cale_imagine,&L,H_imagine,W_imagine);
       unsigned int *fg,*fr, *fb, WxH_imagine;
       WxH_imagine=W_imagine*H_imagine;
       double f_teoretic;
       f_teoretic=(WxH_imagine/256);
       fg=(unsigned int *)calloc(256,sizeof(unsigned int));
       fr=(unsigned int *)calloc(256,sizeof(unsigned int));
       fb=(unsigned int *)calloc(256,sizeof(unsigned int));
       if(fr==NULL||fg==NULL||fb==NULL)
       {
       printf("Eroare la alocarea vectorilor de frecventa pentru calculul chi-patrat.");
       return ;}


       double red,green,blue;

       int i;

       for(i=0;i<WxH_imagine;i++)
       {
           fg[L[i].G]++;
           fr[L[i].R]++;
           fb[L[i].B]++;
       }

     red=green=blue=0;
      for(i=0;i<=255;i++)
      {
          red=red+((fr[i]-f_teoretic)*(fr[i]-f_teoretic)/f_teoretic);
          green=green+((fg[i]-f_teoretic)*(fg[i]-f_teoretic)/f_teoretic);
          blue=blue+((fb[i]-f_teoretic)*(fb[i]-f_teoretic)/f_teoretic);


      }

      printf("\n%.2lf",red);
      printf("\n%.2lf",green);
      printf("\n%.2lf\n\n",blue);
      free(fg);
      free(fr);
      free(fb);
      free(L);


   }


   int main()
{     char nume_imagine_sursa[101];
      char nume_imagine_permutata[ ]="pepperspermutat.bmp"; /* fisierul in care va aparea imaginea permutata */
      char nume_imagine_criptata[101];
      char nume_imagine_decriptata[101];
      char nume_fisier_text[101];

      printf("Numele fisierului ce contine imaginea sursa este: ");
      fgets(nume_imagine_sursa,101,stdin);
      nume_imagine_sursa[strlen(nume_imagine_sursa)-1]='\0';

      printf("Numele fisierului ce contine imaginea criptata este: ");
      fgets(nume_imagine_criptata,101,stdin);
      nume_imagine_criptata[strlen(nume_imagine_criptata)-1]='\0';

      printf("Numele fisierului ce contine imaginea decriptata este: ");
      fgets(nume_imagine_decriptata,101,stdin);
      nume_imagine_decriptata[strlen(nume_imagine_decriptata)-1]='\0';

      printf("Numele fisierului text ce contine cheia secreta este: ");
      fgets(nume_fisier_text,101,stdin);
      nume_fisier_text[strlen(nume_fisier_text)-1]='\0';


       /* calculam latimea si inaltimea imaginii sursa pe care le vom folosi ca parametri */
      unsigned int W_imagine, H_imagine,WxH_imagine;
      W_imagine=latime_imagine(nume_imagine_sursa);
      H_imagine=inaltime_imagine(nume_imagine_sursa);
       if(H_imagine==0||W_imagine==0)
       {
           printf("Eroare la deschiderea fisierului sursa la calculul inaltimii sau a latimii");

       }

       WxH_imagine=W_imagine*H_imagine;


      /* criptarea */
      pixel *L;/*vectorul in care vom pastra imaginea initiala (peppersinitial.bmp) liniarizata */
      liniarizare_imagine(nume_imagine_sursa,&L,H_imagine,W_imagine);


      unsigned int *R; /* in R vom pastra numerele aleatoare generate cu ajutorul functiei XORSHIFT32 */
      XORSHIFT32(&R,nume_fisier_text,WxH_imagine);


      unsigned int *D; /*permutarea  folosind algoritmul lui Durstenfeld*/
      Durstenfeld(&D,R,WxH_imagine);


      pixel *Lpermutat; /*imaginea cu pixelii permutati in forma liniarizata*/
      permutare_pixeli(D,L,&Lpermutat,WxH_imagine);
      /* verificam daca a fost realizata permutarea pixelilor */
      imagine_permutata(nume_imagine_sursa,nume_imagine_permutata,D,L,Lpermutat,W_imagine,H_imagine,WxH_imagine);

      pixel *C; /* imaginea criptata in forma liniarizata */
      criptare(nume_fisier_text,Lpermutat,R,&C,W_imagine,H_imagine,nume_imagine_sursa,nume_imagine_criptata);

      /*decriptarea */
      pixel *Dec; /* imaginea decriptata */
      decriptare(C,&Dec,R,W_imagine,H_imagine,nume_imagine_sursa, nume_imagine_decriptata,nume_fisier_text);


      /* afisarea testului chi_patrat pentru imaginea initiala(peppers.bmp) */
      printf("Valorile testului chi_patrat pentru imaginea in clar sunt:");
      test_chi_patrat(W_imagine,H_imagine,nume_imagine_sursa);

      /* afisarea testului chi_patrat pentru imaginea criptata*/
      printf("Valorile testului chi_patrat pentru imaginea criptata sunt:");
      test_chi_patrat(W_imagine,H_imagine,nume_imagine_criptata);

      free(L);
      free(R);
      free(D);
      free(Lpermutat);
      free(C);
      free(Dec);

    return 0;
}
