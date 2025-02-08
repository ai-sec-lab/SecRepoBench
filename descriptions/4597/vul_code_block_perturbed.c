           gtemplate *new;
           g2int l_index,i,j,k,l;

           l_index=getpdsindex(number);
           if (l_index == -1) return(0);

           new=getpdstemplate(number);
           if (new == NULL) return NULL;

           if ( ! new->needext ) return(new);

           if ( number == 3 ) {
              new->extlen=list[26];
              new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
              for (i=0;i<new->extlen;i++) {
                 new->ext[i]=1;
              }
           }
           else if ( number == 4 ) {
              new->extlen=list[25];
              new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
              for (i=0;i<new->extlen;i++) {
                 new->ext[i]=1;
              }
           }
           else if ( number == 8 ) {
              if ( list[21] > 1 ) {
                 new->extlen=(list[21]-1)*6;
                 new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
                 for (j=2;j<=list[21];j++) {
                    l=(j-2)*6;
                    for (k=0;k<6;k++) {
                       new->ext[l+k]=new->map[23+k];
                    }
                 }
              }
           }
           else if ( number == 9 ) {
              if ( list[28] > 1 ) {
                 new->extlen=(list[28]-1)*6;
                 new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
                 for (j=2;j<=list[28];j++) {
                    l=(j-2)*6;
                    for (k=0;k<6;k++) {
                       new->ext[l+k]=new->map[30+k];
                    }
                 }
              }
           }
           else if ( number == 10 ) {
              if ( list[22] > 1 ) {
                 new->extlen=(list[22]-1)*6;
                 new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
                 for (j=2;j<=list[22];j++) {
                    l=(j-2)*6;
                    for (k=0;k<6;k++) {
                       new->ext[l+k]=new->map[24+k];
                    }
                 }
              }
           }
           else if ( number == 11 ) {
              if ( list[24] > 1 ) {
                 new->extlen=(list[24]-1)*6;
                 new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
                 for (j=2;j<=list[24];j++) {
                    l=(j-2)*6;
                    for (k=0;k<6;k++) {
                       new->ext[l+k]=new->map[26+k];
                    }
                 }
              }
           }
           else if ( number == 12 ) {
              if ( list[23] > 1 ) {
                 new->extlen=(list[23]-1)*6;
                 new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
                 for (j=2;j<=list[23];j++) {
                    l=(j-2)*6;
                    for (k=0;k<6;k++) {
                       new->ext[l+k]=new->map[25+k];
                    }
                 }
              }
           }
           else if ( number == 13 ) {
              new->extlen=(list[37] > 1 ? ((list[37]-1)*6) : 0)+list[26];
              new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
              if ( list[37] > 1 ) {
                 for (j=2;j<=list[37];j++) {
                    l=(j-2)*6;
                    for (k=0;k<6;k++) {
                       new->ext[l+k]=new->map[39+k];
                    }
                 }
              }
              l=(list[37]-1)*6;
              if ( l<0 ) l=0;
              for (i=0;i<list[26];i++) {
                new->ext[l+i]=1;
              }
           }
           else if ( number == 14 ) {
              new->extlen=(list[36] > 1 ? ((list[36]-1)*6) : 0) +list[25];
              new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
              if ( list[36] > 1 ) {
                 for (j=2;j<=list[36];j++) {
                    l=(j-2)*6;
                    for (k=0;k<6;k++) {
                       new->ext[l+k]=new->map[38+k];
                    }
                 }
              }
              l=(list[36]-1)*6;
              if ( l<0 ) l=0;
              for (i=0;i<list[25];i++) {
                new->ext[l+i]=1;
              }
           }
           else if ( number == 30 ) {
              new->extlen=list[4]*5;
              new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
              for (i=0;i<list[4];i++) {
                 l=i*5;
                 new->ext[l]=2;
                 new->ext[l+1]=2;
                 new->ext[l+2]=1;
                 new->ext[l+3]=1;
                 new->ext[l+4]=4;
              }
           }
           else if ( number == 31 ) {
              new->extlen=list[4]*5;
              new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
              for (i=0;i<list[4];i++) {
                 l=i*5;
                 new->ext[l]=2;
                 new->ext[l+1]=2;
                 new->ext[l+2]=2;
                 new->ext[l+3]=1;
                 new->ext[l+4]=4;
              }
           }
           else if ( number == 42 ) {
              if ( list[22] > 1 ) {
                 new->extlen=(list[22]-1)*6;
                 new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
                 for (j=2;j<=list[22];j++) {
                    l=(j-2)*6;
                    for (k=0;k<6;k++) {
                       new->ext[l+k]=new->map[24+k];
                    }
                 }
              }
           }
           else if ( number == 43 ) {
              if ( list[25] > 1 ) {
                 new->extlen=(list[25]-1)*6;
                 new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
                 for (j=2;j<=list[25];j++) {
                    l=(j-2)*6;
                    for (k=0;k<6;k++) {
                       new->ext[l+k]=new->map[27+k];
                    }
                 }
              }
           }
           else if ( number == 32 ) {
              new->extlen=list[9]*5; /* ERO: was 10, but wrong given the below loop */
              new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
              for (i=0;i<list[9];i++) {
                 l=i*5;
                 new->ext[l]=2;
                 new->ext[l+1]=2;
                 new->ext[l+2]=2;
                 new->ext[l+3]=-1;
                 new->ext[l+4]=-4;
              }
           }
           else if ( number == 46 ) {
              if ( list[27] > 1 ) {
                 new->extlen=(list[27]-1)*6;
                 new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
                 for (j=2;j<=list[27];j++) {
                    l=(j-2)*6;
                    for (k=0;k<6;k++) {
                       new->ext[l+k]=new->map[29+k];
                    }
                 }
              }
           }
           else if ( number == 47 ) {
              if ( list[30] > 1 ) {
                 new->extlen=(list[30]-1)*6;
                 new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
                 for (j=2;j<=list[30];j++) {
                    l=(j-2)*6;
                    for (k=0;k<6;k++) {
                       new->ext[l+k]=new->map[32+k];
                    }
                 }
              }
           }
           else if ( number == 51 ) {
              new->extlen=list[15]*11;
              new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
              for (i=0;i<list[15];i++) {
                 l=i*6;
                 new->ext[l]=1;
                 new->ext[l+1]=1;
                 new->ext[l+2]=-1;
                 new->ext[l+3]=-4;
                 new->ext[l+4]=-1;
                 new->ext[l+5]=-4;
              }
           }
           else if ( number == 33 ) {
              new->extlen=list[9];
              new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
              for (i=0;i<new->extlen;i++) {
                 new->ext[i]=1;
              }
           }
           else if ( number == 34 ) {
              new->extlen=(list[24] > 1 ? ((list[24]-1)*6) : 0)+list[9];
              new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
              if ( list[24] > 1 ) {
                 for (j=2;j<=list[24];j++) {
                    l=(j-2)*6;
                    for (k=0;k<6;k++) {
                       new->ext[l+k]=new->map[26+k];
                    }
                 }
              }
              l=(list[24]-1)*6;
              if ( l<0 ) l=0;
              for (i=0;i<list[9];i++) {
                new->ext[l+i]=1;
              }
           }
           else if ( number == 53 ) {
              new->extlen=list[3];
              new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
              for (i=0;i<new->extlen;i++) {
                 new->ext[i]=1;
              }
           }
           else if ( number == 54 ) {
              new->extlen=list[3];
              new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
              for (i=0;i<new->extlen;i++) {
                 new->ext[i]=1;
              }
           }
           else if ( number == 91 ) {
              new->extlen=(list[28] > 1 ? ((list[28]-1)*6) : 0)+list[15];
              new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
              if ( list[28] > 1 ) {
                 for (j=2;j<=list[28];j++) {
                    l=(j-2)*6;
                    for (k=0;k<6;k++) {
                       new->ext[l+k]=new->map[30+k];
                    }
                 }
              }
              l=(list[29]-1)*6;
              if ( l<0 ) l=0;
              for (i=0;i<list[15];i++) {
                new->ext[l+i]=1;
              }
             }
// PDT 4.57  (10/07/2015)
           else if ( number == 57 ) {
              new->extlen=list[6]*15;
              new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
              for (i=0;i<list[6];i++) {
                 l=i*15;
                 new->ext[l]=1;
                 new->ext[l+1]=-4;
                 new->ext[l+2]=1;
                 new->ext[l+3]=1;
                 new->ext[l+4]=1;
                 new->ext[l+5]=2;
                 new->ext[l+6]=1;
                 new->ext[l+7]=1;
                 new->ext[l+8]=-4;
                 new->ext[l+9]=1;
                 new->ext[l+10]=-1;
                 new->ext[l+11]=-4;
                 new->ext[l+12]=1;
                 new->ext[l+13]=-1;
                 new->ext[l+14]=-4;
              }
           }
// PDT 4.61  (10/07/2015)
           else if ( number == 61 ) {
              if ( list[30] > 1 ) {
                 new->extlen=(list[30]-1)*6;
                 new->ext=(g2int *)malloc(sizeof(g2int)*new->extlen);
                 for (j=2;j<=list[30];j++) {
                    l=(j-2)*6;
                    for (k=0;k<6;k++) {
                       new->ext[l+k]=new->map[32+k];
                    }
                 }
              }
           }

           return(new);
