
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <fat.h>

#include "giddy3.h"
#include "render.h"
#include "samples.h"
#include "titles.h"

BOOL audioavailable = FALSE;

BOOL pleft=FALSE, pright=FALSE, pjump=FALSE, lpjump=FALSE, spacehit=FALSE, enterhit=FALSE;
BOOL paused;
extern BOOL ignorejump;
BOOL musicenabled = TRUE;

extern int sfxvol, musicvol, sfxvolopt, musicvolopt, endingstate;

extern struct what_is_giddy_doing gid;
extern int fgx, fgy, titlestate;

int what_are_we_doing = WAWD_TITLES;

void mixaudio( void );

BOOL musicon = TRUE;

void save_options( void )
{
  FILE *f;

  f = fopen( "/apps/giddy3/settings.txt", "w" );
  if( !f ) return;

  fprintf( f, "sfxvol %d\n", sfxvolopt );
  fprintf( f, "musicvol %d\n", musicvolopt );
  fclose( f );
}

void load_options( void )
{
  FILE *f;
  int i;
  char ltmp[80];

  f = fopen( "/apps/giddy3/settings.txt", "r" );
  if( !f ) return;

  while( !feof( f ) )
  {
    if( fgets( ltmp, 80, f ) == NULL )
      break;

    if( strncmp( ltmp, "sfxvol ", 7 ) == 0 )
    {
      i = atoi( &ltmp[7] );
      if( ( i >= 0 ) && ( i <= 8 ) )
      {
        sfxvolopt = i;
        sfxvol = (sfxvolopt * ((MIX_MAX_VOLUME*5)/6))/8;
      }
      continue;
    }

    if( strncmp( ltmp, "musicvol ", 9 ) == 0 )
    {
      i = atoi( &ltmp[9] );
      if( ( i >= 0 ) && ( i <= 8 ) )
      {
        musicvolopt = i;
        musicvol = (musicvolopt * ((MIX_MAX_VOLUME*5)/6))/8;
      }
      continue;
    }

  }    

  fclose( f );
}

BOOL init( void )
{
  video_pre_init();
	fatInitDefault();

	printf( "\x1b[2J\x1b[2;0H\x1b[32;1m  Giddy 3: Reasonably Special Edition. Wii version 1.5\n\n");
  printf( "  Hold the WiiMote like a NES pad!\n\n" );

  printf( "\x1b[37;1m  NOTE: This game is freely distributable (GPL v2)\n" );

  printf( "\x1b[36;1m  Loading options..." );
  load_options();

  printf( "OK\n  Loading sounds..." );
  initsounds();
  loadsounds();

  printf( "OK\n  Launching!" );
  if( !render_init() ) return FALSE;

  titlestate = 0;
  what_are_we_doing = WAWD_TITLES;
  return TRUE;
}

void shut( void )
{
  killtune();
  freesounds();
  sleep( 5 );
}

int main( int argc, char *argv[] )
{
  if( init() )
  {
    u32 wpaddown, wpadheld;
    BOOL done;

    paused = FALSE;
    enterhit = FALSE;
    spacehit = FALSE;

    done = FALSE;
    while( !done )
    {
      timing();
      mixaudio();
      PAD_ScanPads();

      wpadheld = PAD_ButtonsHeld(0);
      wpaddown = PAD_ButtonsDown(0);

      switch( what_are_we_doing )
      {
				case WAWD_ENDING:
          if( wpaddown & (PAD_BUTTON_A|PAD_BUTTON_X) )
				  {
  					if( endingstate < 9 )
	  					endingstate = 11;
					}
					break;

				case WAWD_TITLES:
          if( wpaddown & (PAD_BUTTON_A|PAD_BUTTON_X) )
            go_menus();
          break;
        
        case WAWD_MENU:
          if( wpaddown & PAD_BUTTON_RIGHT ) menu_up();
          if( wpaddown & PAD_BUTTON_LEFT )  menu_down();
          if( wpaddown & PAD_BUTTON_UP )    menu_left();
          if( wpaddown & PAD_BUTTON_DOWN )  menu_right();
          if( wpaddown & PAD_BUTTON_A )     done |= menu_do();
          if( wpaddown & PAD_TRIGGER_Z )  done = TRUE;
          break;
        
        case WAWD_GAME:
          pleft  = (wpadheld & PAD_BUTTON_UP)!=0;
          pright = (wpadheld & PAD_BUTTON_DOWN)!=0;

          if( wpadheld & PAD_BUTTON_A )
          {
            if( !lpjump )
            {
              pjump = TRUE;
              lpjump = TRUE;
            }
          } else {
            lpjump = FALSE;
            pjump = FALSE;
          }

/*
          if( pjump )
          {
            if( ignorejump ) pjump = FALSE;
          } else {
            ignorejump = FALSE;
          }
*/

          if( wpaddown & PAD_BUTTON_X ) spacehit = TRUE;
          if( wpaddown & PAD_BUTTON_Y ) enterhit = TRUE;
                  
          if( wpaddown & PAD_BUTTON_START )
          {
            paused = !paused;
            if( paused ) giddy_say( "Paused" );
          }
          
          /*if( wpaddown & PAD_BUTTON_MINUS )
            musicenabled = !musicenabled;*/

          if( wpaddown & PAD_TRIGGER_Z )
          {
            killtune();
            titlestate = 0;
            what_are_we_doing = WAWD_TITLES;
          }
          break;
      }

      switch( what_are_we_doing )
      {
        case WAWD_TITLES:
        case WAWD_MENU:
          done |= render_titles();
          break;

        case WAWD_GAME:
          done |= render();
          break;
			
			  case WAWD_ENDING:
					done |= render_ending();
				  break;
      }

      flip();
    }
  }
  shut();
  return 0;
}
