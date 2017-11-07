#include "atl-head.h"
#include "atl-mydb.h"

void brutalis_wars()
{ 
UR_OBJECT user,u;
RM_OBJECT rm;
int vec,oldway,accel,i,cnt,nah;
/*
userd_memory=get_memory_usage();
sprintf(texthb,"Memory: %d bytes. Delta: %d\n",userd_memory,userd_memory-last_memory);
write_room(testroom,texthb);
last_memory=userd_memory;
*/
 if (tsec%2==1) {
   for(rm=room_first;rm!=NULL;rm=rm->next)
   for(i=0;i<MPVM;i++) if (rm->predmet[i]>-1 && predmet[rm->predmet[i]]->function==24) {
     if (0<=rm->dur[i] && rm->dur[i]<10000) {
       if (tsec%4==0) rm->dur[i]=dec_dur(rm->dur[i],1);
       if (rm->dur[i]==0) {
         if (predmet[rm->predmet[i]]->rdestroy!=NULL) {
           sprintf(text,"%s\n",parse_phrase(predmet[rm->predmet[i]]->rdestroy,NULL,NULL,NULL,0));
           write_room(rm,text);
          }
         rm->predmet[i]=-1;
         rm->dur[i]=0;
         continue;
        }
      }
     else if (rm->dur[i]>9999) {
       for (u=user_first;u!=NULL;u=u->next)
       if (!u->login && u->type!=CLONE_TYPE && u->zaradeny
       && u->socket==(rm->dur[i]%10000)) {
         if (tsec%30==0) {
           rm->dur[i]-=10000;
           if (rm->dur[i]<10000) {
             if (predmet[rm->predmet[i]]->rdestroy!=NULL) {
               sprintf(text,"%s\n",parse_phrase(predmet[rm->predmet[i]]->rdestroy,NULL,NULL,NULL,0));
               write_room(rm,text);
              }
             rm->predmet[i]=-1;
             rm->dur[i]=0;
             break;
            }
          }
  	 if (u->room==rm) break;
         if (u->room->group!=rm->group) { rm->dur[i]=abs_dur(rm->dur[i]); break; }
  	 else if (is_free_in_room(u->room)>-1 && (int)(rand()%3)==0) {
           if (predmet[rm->predmet[i]]->searchphr!=NULL) {
             sprintf(text,"%s\n",parse_phrase(predmet[rm->predmet[i]]->searchphr,u,u,u->room,0));
             write_room(rm,text);
            }
           if (predmet[rm->predmet[i]]->ujoinphr!=NULL) {
             sprintf(text,"%s\n",parse_phrase(predmet[rm->predmet[i]]->ujoinphr,u,u,u->room,0));
             write_user(u,text);
	    }
           if (predmet[rm->predmet[i]]->rjoinphr!=NULL) {
             sprintf(text,"%s\n",parse_phrase(predmet[rm->predmet[i]]->rjoinphr,u,u,u->room,10));
             write_room_except(u->room,text,u);
	    }
	   put_in_room(u->room,rm->predmet[i],rm->dur[i]);
	   rm->predmet[i]=-1;
	   rm->dur[i]=0;
	   break;
          }
        }
      }
     else if (rm->dur[i]<-9999) {
       nah=(-rm->dur[i]%10000);
       u=get_user_by_sock(-rm->dur[i]%10000);
       if (u->room!=rm) {
         rm->dur[i]=abs_dur(rm->dur[i]);
         continue;
        }
       vec=rm->predmet[i];
       nah=0;
       if (predmet[vec]->firerate<0 && tsec%-predmet[vec]->firerate==i%-predmet[vec]->firerate) nah=1;
       else if (predmet[vec]->firerate>0 && rand()%100<predmet[vec]->firerate) nah=1;
       if (nah==1) {
         if (predmet[vec]->ustart!=NULL) {
           sprintf(text,"%s\n",parse_phrase(predmet[vec]->ustart,u,NULL,u->room,0));
           write_user(u,text);
          }
         if (predmet[vec]->rstart!=NULL) {
           sprintf(text,"%s\n",parse_phrase(predmet[vec]->rstart,u,NULL,u->room,10));
           write_room_except(u->room,text,u);
          }
	 rm->dur[i]=dec_dur(rm->dur[i],4+rand()%2);
         hurt(NULL,u,vec);
         if (u->health<10) rm->dur[i]=abs_dur(rm->dur[i]);
         do_alt_funct(u,vec);
	 if (rm->dur[i]==0) {
  	   if (predmet[vec]->udestroy!=NULL) {
	     sprintf(text,"%s\n",parse_phrase(predmet[vec]->udestroy,u,NULL,u->room,0));
	     write_user(u,text);
	    }
	   if (predmet[vec]->rdestroy!=NULL) {
	     sprintf(text,"%s\n",parse_phrase(predmet[vec]->rdestroy,u,NULL,u->room,10));
	     write_room_except(u->room,text,u);
	    }
	   rm->predmet[i]=-1;
	   rm->dur[i]=0;
	  }
        }
      }
    }
  }

for(user=user_first;user!=NULL;user=user->next)
if (!user->login && user->type==USER_TYPE && user->room!=NULL && user->dead==0) {
  if (april) {
    if (april & 16) {
      user->cloak=1+rand()%8;
      if (user->cloak==6) user->cloak=8;
     }
    if (april & 32) user->vis=rand()%2;
    if (april & 64) user->last_login=time(0)-rand()%86400;
    if (april & 256) {
      user->cloak=0;
      user->vis=1;
      user->last_login=time(0);
     }
   }
  if (user->attacking>0) user->attacking--;
  if (tsec%2==1 && user->health<100 && user->heal>0) { user->heal--; user->health++; }
  if (tsec%2==0) {
    if (user->health<100) user->health++;
    if (user->health<10) {
      user->health-=2;
      if (user->health==1) user->health--;
      if (user->health==0) user->dead=14;
     }
   }
  if (user->target>-1) { /* zombie-like entities */
    u=get_user_by_sock(user->target);
    if (u==NULL || (u!=NULL && (u->room!=user->room || u->dead))) {
      user->target=-1;
      continue;
     }
    cnt=0;
    for(i=0;i<MPVM;i++) {
      if (user->room->predmet[i]>-1 && user->room->dur[i]%10000==user->socket
       && predmet[user->room->predmet[i]]->function==24) {
         cnt++;
         vec=user->room->predmet[i];
         accel=0;
	 if (predmet[vec]->firerate<0 && tsec%-predmet[vec]->firerate==i%-predmet[vec]->firerate) accel=1;
	 else if (predmet[vec]->firerate>0 && rand()%100<predmet[vec]->firerate) accel=1;
	 if (accel==1) {
	   if (predmet[vec]->ustart!=NULL) {
	     sprintf(text,"%s\n",parse_phrase(predmet[vec]->ustart,u,NULL,u->room,0));
	     write_user(u,text);
	    }
	   if (predmet[vec]->rstart!=NULL) {
	     sprintf(text,"%s\n",parse_phrase(predmet[vec]->rstart,u,NULL,u->room,10));
	     write_room_except(u->room,text,u);
	    }
	   hurt(user,u,vec);
	   if (u->dead || u->health<10) { user->target=-1; break; }
	   do_alt_funct(u,vec);
	   user->room->dur[i]-=(4+rand()%2)*10000;
	   if (user->room->dur[i]<10000) {
  	     if (predmet[vec]->udestroy!=NULL) {
	       sprintf(text,"%s\n",parse_phrase(predmet[vec]->udestroy,user,u,user->room,0));
	       write_user(user,text);
	      }
	     if (predmet[vec]->rdestroy!=NULL) {
	       sprintf(text,"%s\n",parse_phrase(predmet[vec]->rdestroy,user,u,user->room,10));
	       write_room_except(user->room,text,user);
	      }
	     user->room->predmet[i]=-1;
	     user->room->dur[i]=0;
	    }
	  }
        }
     }
    if (cnt==0) user->target=-1;
   }
  if (user->room->group==4 || (user->into!=NULL && user->into->group==4)) { /* brutalis walking engine */
    if (user->into!=NULL || user->from!=NULL) {
/*      sprintf(text,"%d",user->way);
      write_user(user,text);*/
      vec=get_vehicle(user);
      oldway=user->way;
      if (user->predmet[HANDS+2]>-1) accel=predmet[user->predmet[HANDS+2]]->seconddur;
      else accel=0;
      if (vec>-1) user->way+=10+predmet[vec]->seconddur;
      else user->way+=10+accel;
      if (oldway<0 && user->way>=0) {
        if (user->into==NULL && user->from!=NULL) {
          user->from=NULL;
          if (vec>-1 && predmet[vec]->userphr!=NULL) {
            sprintf(text,"~FG%s\n",parse_phrase(predmet[vec]->userphr,user,NULL,NULL,0));
            write_user(user,text);
           }
          else {
            sprintf(text,"~FGZastal%s si %s.\n",pohl(user,"","a"),user->room->where);
            write_user(user,text);
           }
          if (vec>-1 && predmet[vec]->roomphr!=NULL) {
            sprintf(text,"~FG%s\n",parse_phrase(predmet[vec]->roomphr,user,NULL,NULL,10));
            write_room_except(user->room,text,user);
           }
/*          else {
            sprintf(text,"~FG%s tu zastal%s.\n",pohl(user,"","a"),user->room->where);
            write_room_except(user->room,text,user);
           }*/
         }
        if (user->into!=NULL && user->from!=NULL) {
          user->from=NULL;
         }
       }
      if (user->way>49) {
        if (user->into==NULL) { user->way=0; return; }
        move_vehicle(user,user->into,1);
        user->from=user->room;
        user->room=user->into;
        user->reveal=20+rand()%10;
        user->hidden=0;
        look(user);
        reset_access(user->from);
        if (user->statline==CHARMODE) show_statline(user);
        if (vec>-1 && predmet[vec]->victimphr!=NULL) {
          sprintf(text,"~FG<- %s\n",parse_phrase(predmet[vec]->victimphr,user,NULL,user->from,1));
          write_room_except(user->room,text,user);
         }
        else {
          sprintf(text,"~FG<- %s%s~RS~FW~FG %s.\n",/*teamcolor[user->team]*/"",user->name,user->in_phrase);
          write_room_except(user->room,text,user);
         }
        if (vec>-1) {
          sprintf(text,"~FG-> ~FG%s%s~RS~FG sa stratil%s z dohladu.\n",/*teamcolor[user->team]*/"",user->name,pohl(user,"","a"));
          write_room_except(user->from,text,user);
         }
        else {
          sprintf(text,"~FG-> %s%s~RS~FW~FG %s.\n",/*teamcolor[user->team]*/"",user->name,expand_outphr(user,user->room));
          write_room_except(user->from,text,user);
         }
        user->into=NULL;
        user->way=-40;
        if (user->predmet[HANDS+2]>-1)  {
          user->dur[HANDS+2]--;
          if (user->dur[HANDS+2]==0) {
            if (predmet[user->predmet[HANDS+2]]->udestroy!=NULL) {
              sprintf(text,"%s\n",parse_phrase(predmet[user->predmet[HANDS+2]]->udestroy,user,NULL,NULL,0));
              write_user(user,text);
             }
            if (predmet[user->predmet[HANDS+2]]->rdestroy!=NULL) {
              sprintf(text,"%s\n",parse_phrase(predmet[user->predmet[HANDS+2]]->rdestroy,user,NULL,NULL,10));
              write_room_except(user->room,text,user);
             }
           }
         }
       }
     }
    else user->way=0;
   }
 }
} 

