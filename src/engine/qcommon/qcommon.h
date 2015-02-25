/*
===========================================================================

Daemon GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

This file is part of the Daemon GPL Source Code (Daemon Source Code).

Daemon Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Daemon Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Daemon Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Daemon Source Code is also subject to certain additional terms.
You should have received a copy of these additional terms immediately following the
terms and conditions of the GNU General Public License which accompanied the Daemon
Source Code.  If not, please request a copy in writing from id Software at the address
below.

If you have questions concerning this license or the applicable additional terms, you
may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville,
Maryland 20850 USA.

===========================================================================
*/

// qcommon.h -- definitions common between client and server, but not game.or ref modules
#ifndef QCOMMON_H_
#define QCOMMON_H_

#include "../../common/cm/cm_public.h"
#include "cvar.h"

//============================================================================

//
// msg.c
//
typedef struct
{
    qboolean allowoverflow; // if false, do a Com_Error
    qboolean overflowed; // set to true if the buffer size failed (with allowoverflow set)
    qboolean oob; // set to true if the buffer size failed (with allowoverflow set)
    byte     *data;
    int      maxsize;
    int      cursize;
    int      uncompsize; // NERVE - SMF - net debugging
    int      readcount;
    int      bit; // for bitwise reads and writes
} msg_t;

void MSG_Init( msg_t *buf, byte *data, int length );
void MSG_InitOOB( msg_t *buf, byte *data, int length );
void MSG_Clear( msg_t *buf );
void *MSG_GetSpace( msg_t *buf, int length );
void MSG_WriteData( msg_t *buf, const void *data, int length );
void MSG_Bitstream( msg_t *buf );
void MSG_Uncompressed( msg_t *buf );

// TTimo
// copy a msg_t in case we need to store it as is for a bit
// (as I needed this to keep an msg_t from a static var for later use)
// sets data buffer as MSG_Init does prior to do the copy
void MSG_Copy( msg_t *buf, byte *data, int length, msg_t *src );

struct usercmd_s;

struct entityState_s;

struct playerState_s;

void  MSG_WriteBits( msg_t *msg, int value, int bits );

void  MSG_WriteChar( msg_t *sb, int c );
void  MSG_WriteByte( msg_t *sb, int c );
void  MSG_WriteShort( msg_t *sb, int c );
void  MSG_WriteLong( msg_t *sb, int c );
void  MSG_WriteFloat( msg_t *sb, float f );
void  MSG_WriteString( msg_t *sb, const char *s );
void  MSG_WriteBigString( msg_t *sb, const char *s );
void  MSG_WriteAngle16( msg_t *sb, float f );

void  MSG_BeginReading( msg_t *sb );
void  MSG_BeginReadingOOB( msg_t *sb );
void  MSG_BeginReadingUncompressed( msg_t *msg );

int   MSG_ReadBits( msg_t *msg, int bits );

int   MSG_ReadChar( msg_t *sb );
int   MSG_ReadByte( msg_t *sb );
int   MSG_ReadShort( msg_t *sb );
int   MSG_ReadLong( msg_t *sb );
float MSG_ReadFloat( msg_t *sb );
char  *MSG_ReadString( msg_t *sb );
char  *MSG_ReadBigString( msg_t *sb );
char  *MSG_ReadStringLine( msg_t *sb );
float MSG_ReadAngle16( msg_t *sb );
void  MSG_ReadData( msg_t *sb, void *buffer, int size );
int   MSG_LookaheadByte( msg_t *msg );

void  MSG_WriteDeltaUsercmd( msg_t *msg, usercmd_t *from, usercmd_t *to );
void  MSG_ReadDeltaUsercmd( msg_t *msg, usercmd_t *from, usercmd_t *to );

void  MSG_WriteDeltaEntity( msg_t *msg, struct entityState_s *from, struct entityState_s *to, qboolean force );
void  MSG_ReadDeltaEntity( msg_t *msg, entityState_t *from, entityState_t *to, int number );

void  MSG_WriteDeltaPlayerstate( msg_t *msg, struct playerState_s *from, struct playerState_s *to );
void  MSG_ReadDeltaPlayerstate( msg_t *msg, struct playerState_s *from, struct playerState_s *to );

//============================================================================

/*
==============================================================

NET

==============================================================
*/

#define NET_ENABLEV4        0x01
#define NET_ENABLEV6        0x02
// if this flag is set, always attempt IPv6 connections instead of IPv4 if a v6 address is found.
#define NET_PRIOV6          0x04
// disables IPv6 multicast support if set.
#define NET_DISABLEMCAST    0x08

// right type anyway || ( DUAL && proto enabled && ( other proto disabled || appropriate IPv6 pref ) )
#define NET_IS_IPv4( type ) ( ( type ) == NA_IP  || ( ( type ) == NA_IP_DUAL && ( net_enabled->integer & NET_ENABLEV4 ) && ( ( ~net_enabled->integer & NET_ENABLEV6) || ( ~net_enabled->integer & NET_PRIOV6 ) ) ) )
#define NET_IS_IPv6( type ) ( ( type ) == NA_IP6 || ( ( type ) == NA_IP_DUAL && ( net_enabled->integer & NET_ENABLEV6 ) && ( ( ~net_enabled->integer & NET_ENABLEV4) || (  net_enabled->integer & NET_PRIOV6 ) ) ) )
// if NA_IP_DUAL, get the preferred type (falling back on NA_IP)
#define NET_TYPE( type )    ( NET_IS_IPv4( type ) ? NA_IP : NET_IS_IPv6( type ) ? NA_IP6 : ( ( type ) == NA_IP_DUAL ) ? NA_IP : ( type ) )

#define PACKET_BACKUP       32 // number of old messages that must be kept on client and
// server for delta comrpession and ping estimation
#define PACKET_MASK         ( PACKET_BACKUP - 1 )

#define MAX_PACKET_USERCMDS 32 // max number of usercmd_t in a packet

#define PORT_ANY            0

#define MAX_MASTER_SERVERS  5

// RF, increased this, seems to keep causing problems when set to 64, especially when loading
// a savegame, which is hard to fix on that side, since we can't really spread out a loadgame
// among several frames
//#define   MAX_RELIABLE_COMMANDS   64          // max string commands buffered for restransmit
//#define   MAX_RELIABLE_COMMANDS   128         // max string commands buffered for restransmit
#define MAX_RELIABLE_COMMANDS 256 // bigger!

typedef enum
{
  NA_BOT,
  NA_BAD, // an address lookup failed
  NA_LOOPBACK,
  NA_BROADCAST,
  NA_IP,
  NA_IP6,
  NA_IP_DUAL,
  NA_MULTICAST6,
  NA_UNSPEC
} netadrtype_t;

typedef enum
{
  NS_CLIENT,
  NS_SERVER
} netsrc_t;

// maximum length of an IPv6 address string including trailing '\0'
#define NET_ADDR_STR_MAX_LEN 48

// maximum length of an formatted IPv6 address string including port and trailing '\0'
// format [%s]:%hu - 48 for %s (address), 3 for []: and 5 for %hu (port number, max value 65535)
#define NET_ADDR_W_PORT_STR_MAX_LEN ( NET_ADDR_STR_MAX_LEN + 3 + 5 )

typedef struct
{
    netadrtype_t   type;

    byte           ip[ 4 ];
    byte           ip6[ 16 ];

    unsigned short port; // port which is in use
    unsigned short port4, port6; // ports to choose from
    unsigned long  scope_id; // Needed for IPv6 link-local addresses
} netadr_t;

extern cvar_t       *net_enabled;

void       NET_Init( void );
void       NET_Shutdown( void );
void       NET_Restart_f( void );
void       NET_Config( qboolean enableNetworking );

void       NET_SendPacket( netsrc_t sock, int length, const void *data, netadr_t to );
void QDECL NET_OutOfBandPrint( netsrc_t net_socket, netadr_t adr, const char *format, ... ) PRINTF_LIKE(3);
void QDECL NET_OutOfBandData( netsrc_t sock, netadr_t adr, byte *format, int len );

qboolean   NET_CompareAdr( netadr_t a, netadr_t b );
qboolean   NET_CompareBaseAdr( netadr_t a, netadr_t b );
qboolean   NET_IsLocalAddress( netadr_t adr );
const char *NET_AdrToString( netadr_t a );
const char *NET_AdrToStringwPort( netadr_t a );
int        NET_StringToAdr( const char *s, netadr_t *a, netadrtype_t family );
qboolean   NET_GetLoopPacket( netsrc_t sock, netadr_t *net_from, msg_t *net_message );
void       NET_JoinMulticast6( void );
void       NET_LeaveMulticast6( void );

void       NET_Sleep( int msec );

#ifdef HAVE_GEOIP
const char *NET_GeoIP_Country( const netadr_t *a );
#endif

//----(SA)  increased for larger submodel entity counts
#define MAX_MSGLEN           32768 // max length of a message, which may
//#define   MAX_MSGLEN              16384       // max length of a message, which may
// be fragmented into multiple packets
#define MAX_DOWNLOAD_WINDOW  8 // max of eight download frames
#define MAX_DOWNLOAD_BLKSIZE 2048 // 2048 byte block chunks

/*
Netchan handles packet fragmentation and out of order / duplicate suppression
*/

typedef struct
{
    netsrc_t sock;

    int      dropped; // between last packet and previous

    netadr_t remoteAddress;
    int      qport; // qport value to write when transmitting

    // sequencing variables
    int incomingSequence;
    int outgoingSequence;

    // incoming fragment assembly buffer
    int  fragmentSequence;
    int  fragmentLength;
    byte fragmentBuffer[ MAX_MSGLEN ];

    // outgoing fragment buffer
    // we need to space out the sending of large fragmented messages
    qboolean unsentFragments;
    int      unsentFragmentStart;
    int      unsentLength;
    byte     unsentBuffer[ MAX_MSGLEN ];
} netchan_t;

void     Netchan_Init( int qport );
void     Netchan_Setup( netsrc_t sock, netchan_t *chan, netadr_t adr, int qport );

void     Netchan_Transmit( netchan_t *chan, int length, const byte *data );
void     Netchan_TransmitNextFragment( netchan_t *chan );

qboolean Netchan_Process( netchan_t *chan, msg_t *msg );

/*
==============================================================

PROTOCOL

==============================================================
*/

// sent by the server, printed on connection screen, works for all clients
// (restrictions: does not handle \n, no more than 256 chars)
#define PROTOCOL_MISMATCH_ERROR      "ERROR: Protocol Mismatch Between Client and Server. \
The server you are attempting to join is running an incompatible version of the game."

// long version used by the client in diagnostic window
#define PROTOCOL_MISMATCH_ERROR_LONG "ERROR: Protocol Mismatch Between Client and Server.\n\n\
The server you attempted to join is running an incompatible version of the game.\n\
You or the server may be running older versions of the game."

#define GAMENAME_STRING        "unv"

#define PROTOCOL_VERSION       86

#define URI_SCHEME             GAMENAME_STRING "://"
#define URI_SCHEME_LENGTH      ( ARRAY_LEN( URI_SCHEME ) - 1 )

// NERVE - SMF - wolf multiplayer master servers
#ifndef MASTER_SERVER_NAME
# define MASTER_SERVER_NAME    "master.unvanquished.net"
#endif

#ifndef MOTD_SERVER_NAME
# define MOTD_SERVER_NAME      "master.unvanquished.net"
#endif

#define PORT_MASTER             27950
#define PORT_MOTD               27950
#define PORT_SERVER             27960
#define NUM_SERVER_PORTS        4 // broadcast scan this many ports after
// PORT_SERVER so a single machine can
// run multiple servers
// the svc_strings[] array in cl_parse.c should mirror this
//
// server to client
//
enum svc_ops_e
{
  svc_bad,
  svc_nop,
  svc_gamestate,
  svc_configstring, // [short] [string] only in gamestate messages
  svc_baseline, // only in gamestate messages
  svc_serverCommand, // [string] to be executed by client game module
  svc_download, // [short] size [size bytes]
  svc_snapshot,
  svc_EOF,

  // svc_extension follows a svc_EOF, followed by another svc_* ...
  //  this keeps legacy clients compatible.
  svc_extension,
  svc_voip, // not wrapped in USE_VOIP, so this value is reserved.
};

//
// client to server
//
enum clc_ops_e
{
  clc_bad,
  clc_nop,
  clc_move, // [usercmd_t]
  clc_moveNoDelta, // [usercmd_t]
  clc_clientCommand, // [string] message
  clc_EOF,
  clc_voip, // not wrapped in USE_VOIP, so this value is reserved.
};

/*
==============================================================

VIRTUAL MACHINE

==============================================================
*/

// See also vm_traps.h for syscalls common to all VMs

typedef struct vm_s vm_t;

typedef enum
{
  VMI_NATIVE,
  VMI_BYTECODE,
  VMI_COMPILED
} vmInterpret_t;

void VM_Init( void );

vm_t *VM_Create( const char *module, intptr_t ( *systemCalls )( intptr_t * ), vmInterpret_t interpret );

// module should be bare: "cgame", not "cgame.dll", "vm/cgame.qvm"

void           VM_Free( vm_t *vm );
void           VM_Clear( void );
void           VM_Forced_Unload_Start( void );
void           VM_Forced_Unload_Done( void );
vm_t           *VM_Restart( vm_t *vm );

ATTRIBUTE_NO_SANITIZE_ADDRESS intptr_t QDECL VM_Call( vm_t *vm, int callNum, ... );
ATTRIBUTE_NO_SANITIZE_ADDRESS intptr_t QDECL VM_DllSyscall( intptr_t arg, ... );

void           VM_Debug( int level );

void           *VM_ArgPtr( intptr_t intValue );
void           *VM_ExplicitArgPtr( vm_t *vm, intptr_t intValue );

void VM_CheckBlock( intptr_t buf, size_t n, const char *fail );
void VM_CheckBlockPair( intptr_t dest, intptr_t src, size_t dn, size_t sn, const char *fail );

intptr_t       VM_SystemCall( intptr_t *args ); // common system calls

#define VMA(x) VM_ArgPtr(args[ x ])
static INLINE float _vmf( intptr_t x )
{
    floatint_t fi;
    fi.i = ( int ) x;
    return fi.f;
}

#define VMF(x) _vmf(args[ x ])

/*
==============================================================

CMD

Command text buffering and command execution

==============================================================
*/

/*

Any number of commands can be added in a frame, from several different sources.
Most commands come from either keybindings or console line input, but entire text
files can be execed.

*/

void Cbuf_ExecuteText( int exec_when, const char *text );

// this can be used in place of either Cbuf_AddText or Cbuf_InsertText

//===========================================================================

/*

Command execution takes a null terminated string, breaks it into tokens,
then searches for a command or variable that matches the first token.

*/

typedef void ( *xcommand_t )( void );
typedef void ( *xcommand_arg_t )( int );

void     Cmd_AddCommand( const char *cmd_name, xcommand_t function );

// called by the init functions of other parts of the program to
// register commands and functions to call for them.
// The cmd_name is referenced later, so it should not be in temp memory
// if function is NULL, the command will be forwarded to the server
// as a clc_clientCommand instead of executed locally

void Cmd_RemoveCommand( const char *cmd_name );
void Cmd_RemoveCommandsByFunc( xcommand_t function );

void Cmd_CommandCompletion( void ( *callback )( const char *s ) );

typedef void ( *completionFunc_t )( char *args, int argNum );

void Cmd_OnCompleteMatch(const char* s);
void Cmd_AliasCompletion( void ( *callback )( const char *s ) );
void Cmd_DelayCompletion( void ( *callback )( const char *s ) );

void Cmd_SetCommandCompletionFunc( const char *command,
                                   completionFunc_t complete );
void Cmd_CompleteArgument( const char *command, char *args, int argNum );
void Cmd_CompleteCfgName( char *args, int argNum );

// callback with each valid string

void Cmd_PrintUsage( const char *syntax, const char *description );
int  Cmd_Argc( void );
char *Cmd_Argv( int arg );
void Cmd_ArgvBuffer( int arg, char *buffer, int bufferLength );
char *Cmd_Args( void );
char *Cmd_ArgsFrom( int arg );
void Cmd_EscapedArgsBuffer( char* buffer, int bufferLength ); // from index 0
void Cmd_LiteralArgsBuffer( char* buffer, int bufferLength );
const char *Cmd_Cmd( void );
const char *Cmd_Cmd_FromNth( int );

// these all share an output buffer
const char *Cmd_QuoteString( const char *in );
const char *Cmd_UnquoteString( const char *in );

void Cmd_QuoteStringBuffer( const char *in, char *buffer, int size );

// The functions that execute commands get their parameters with these
// functions. Cmd_Argv () will return an empty string, not a NULL
// if arg >= argc, so string operations are always safe.

void Cmd_TokenizeString( const char *text );
void Cmd_LiteralArgsBuffer( char *buffer, int bufferLength );
void Cmd_SaveCmdContext( void );
void Cmd_RestoreCmdContext( void );

/*
==============================================================

FILESYSTEM

No stdio calls should be used by any part of the game, because
we need to deal with all sorts of directory and separator char
issues.
==============================================================
*/

char **FS_ListFiles( const char *directory, const char *extension, int *numfiles );

// directory should not have either a leading or trailing /
// if extension is "/", only subdirectories will be returned
// the returned files will not include any directories or /

void         FS_FreeFileList( char **list );

qboolean     FS_FileExists( const char *file );

int          FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize );
int          FS_GetFileListRecursive( const char* path, const char* extension, char* listBuf, int bufSize );

fileHandle_t FS_FOpenFileWrite( const char *qpath );
fileHandle_t FS_FOpenFileAppend( const char *filename );
fileHandle_t  FS_FCreateOpenPipeFile( const char *filename );

fileHandle_t FS_FOpenFileWriteViaTemporary( const char *qpath );

// will properly create any needed paths and deal with separator character issues

fileHandle_t FS_SV_FOpenFileWrite( const char *filename );
int          FS_SV_FOpenFileRead( const char *filename, fileHandle_t *fp );
void         FS_SV_Rename( const char *from, const char *to );
int          FS_FOpenFileRead( const char *qpath, fileHandle_t *file, qboolean uniqueFILE );

/*
if uniqueFILE is true, then a new FILE will be fopened even if the file
is found in an already open pak file.  If uniqueFILE is false, you must call
FS_FCloseFile instead of fclose, otherwise the pak FILE would be improperly closed
It is generally safe to always set uniqueFILE to true, because the majority of
file IO goes through FS_ReadFile, which Does The Right Thing already.
*/

int FS_Delete( const char *filename );  // only works inside the 'save' directory (for deleting savegames/images)

int FS_Write( const void *buffer, int len, fileHandle_t f );

int FS_Read( void *buffer, int len, fileHandle_t f );

// properly handles partial reads and reads from other dlls

int FS_FCloseFile( fileHandle_t f ); // !0 on error (but errno isn't valid)

// note: you can't just fclose from another DLL, due to MS libc issues

int  FS_ReadFile( const char *qpath, void **buffer );

// returns the length of the file
// a null buffer will just return the file length without loading,
// as a quick check for existence. -1 length == not present
// A 0 byte will always be appended at the end, so string ops are safe.
// the buffer should be considered read-only, because it may be cached
// for other uses.

void FS_ForceFlush( fileHandle_t f );

// forces flush on files we're writing to.

void FS_FreeFile( void *buffer );

// frees the memory returned by FS_ReadFile

void FS_WriteFile( const char *qpath, const void *buffer, int size );

// writes a complete file, creating any subdirectories needed

int FS_filelength( fileHandle_t f );

// doesn't work for files that are opened from a pack file

int FS_FTell( fileHandle_t f );

// where are we?

void       FS_Flush( fileHandle_t f );

void QDECL FS_Printf( fileHandle_t f, const char *fmt, ... ) PRINTF_LIKE(2);

// like fprintf

int FS_Game_FOpenFileByMode( const char *qpath, fileHandle_t *f, fsMode_t mode );

// opens a file for reading, writing, or appending depending on the value of mode

int FS_Seek( fileHandle_t f, long offset, int origin );

// seek on a file (doesn't work for zip files!!!!!!!!)

const char* FS_LoadedPaks();

// Returns a space separated string containing all loaded pk3 files.

bool     FS_LoadPak( const char *name );
void     FS_LoadBasePak();
void     FS_LoadAllMapMetadata();
bool     FS_LoadServerPaks( const char* paks, bool isDemo );

// shutdown and restart the filesystem so changes to fs_gamedir can take effect

qboolean   FS_ComparePaks( char *neededpaks, int len, qboolean dlstring );

void       FS_Rename( const char *from, const char *to );

/*
==============================================================

DOWNLOAD

==============================================================
*/

#include "dl_public.h"

/*
==============================================================

Edit fields and command line history/completion

==============================================================
*/

#define MAX_EDIT_LINE 256
typedef struct
{
    int  cursor;
    int  scroll;
    int  widthInChars;
    char buffer[ MAX_EDIT_LINE ];
} field_t;

// Field_Complete{Key,Team}name
#define FIELD_TEAM            1
#define FIELD_TEAM_SPECTATORS 2
#define FIELD_TEAM_DEFAULT    4

void Field_CompleteKeyname( int flags );
void Field_CompleteTeamname( int flags );

// code point count <-> UTF-8 byte count
int Field_CursorToOffset( field_t *edit );
int Field_ScrollToOffset( field_t *edit );

/*
==============================================================

MISC

==============================================================
*/

// returned by Sys_GetProcessorFeatures
typedef enum
{
  CF_RDTSC = BIT( 0 ),
  CF_MMX = BIT( 1 ),
  CF_MMX_EXT = BIT( 2 ),
  CF_3DNOW = BIT( 3 ),
  CF_3DNOW_EXT = BIT( 4 ),
  CF_SSE = BIT( 5 ),
  CF_SSE2 = BIT( 6 ),
  CF_SSE3 = BIT( 7 ),
  CF_SSSE3 = BIT( 8 ),
  CF_SSE4_1 = BIT( 9 ),
  CF_SSE4_2 = BIT( 10 ),
  CF_ALTIVEC = BIT( 11 ),
  CF_HasHTT = BIT( 12 ),
  CF_HasSerial = BIT( 13 ),
  CF_Is64Bit = BIT( 14 )
} cpuFeatures_t;

// TTimo
// centralized and cleaned, that's the max string you can send to a Com_Printf / Com_DPrintf (above gets truncated)
#define MAXPRINTMSG 4096

void       Info_Print( const char *s );

// *INDENT-OFF*
int QDECL  Com_VPrintf( const char *fmt, va_list argptr ) VPRINTF_LIKE(1);    // conforms to vprintf prototype for print callback passing
void QDECL Com_LogEvent( log_event_t *event, log_location_info_t *location );

void QDECL PRINTF_LIKE(2) Com_Logf( log_level_t level, const char *fmt, ... );
void QDECL Com_Log( log_level_t level, const char* message );

#define    PrintBanner(text) Com_Printf("----- %s -----\n", text );

// *INDENT-ON*
void NORETURN Com_Quit_f( void );
int        Com_Milliseconds( void );
unsigned   Com_BlockChecksum( const void *buffer, int length );
char       *Com_MD5File( const char *filename, int length );
void       Com_MD5Buffer( const char *pubkey, int size, char *buffer, int bufsize );
int        Com_FilterPath( const char *filter, char *name, int casesensitive );
int        Com_RealTime( qtime_t *qtime );
int        Com_GMTime( qtime_t *qtime );
// Com_Time: client gets local time, server gets GMT
#ifdef BUILD_SERVER
#define Com_Time(t) Com_GMTime(t)
#else
#define Com_Time(t) Com_RealTime(t)
#endif
qboolean   Com_SafeMode( void );

qboolean   Com_IsVoipTarget( uint8_t *voipTargets, int voipTargetsSize, int clientNum );

void       Com_StartupVariable( const char *match );
void       Com_SetRecommended( void );
bool       Com_AreCheatsAllowed();
bool       Com_IsClient();
bool       Com_IsDedicatedServer();
bool       Com_ServerRunning();

// checks for and removes command line "+set var arg" constructs
// if match is NULL, all set commands will be executed, otherwise
// only a set with the exact name.  Only used during startup.

extern cvar_t       *com_crashed;

extern cvar_t       *com_developer;
extern cvar_t       *com_speeds;
extern cvar_t       *com_timescale;
extern cvar_t       *com_sv_running;
extern cvar_t       *com_cl_running;
extern cvar_t       *com_version;

extern cvar_t       *com_consoleCommand;

extern cvar_t       *com_ansiColor;

extern cvar_t       *com_unfocused;
extern cvar_t       *com_minimized;

// both client and server must agree to pause
extern cvar_t       *cl_paused;
extern cvar_t       *sv_paused;

extern cvar_t       *cl_packetdelay;
extern cvar_t       *sv_packetdelay;

// com_speeds times
extern int          time_game;
extern int          time_frontend;
extern int          time_backend; // renderer backend time

extern int          com_frameTime;
extern int          com_frameMsec;
extern int          com_hunkusedvalue;

extern qboolean     com_errorEntered;

typedef enum
{
  TAG_FREE,
  TAG_GENERAL,
  TAG_BOTLIB,
  TAG_RENDERER,
  TAG_SMALL,
  TAG_CRYPTO,
  TAG_STATIC
} memtag_t;

/*

--- low memory ----
server vm
server clipmap
---mark---
renderer initialization (shaders, etc)
UI vm
cgame vm
renderer map
renderer models

---free---

temp file loading
--- high memory ---

*/

// Use malloc instead of the zone allocator
static inline void* Z_TagMalloc(size_t size, int tag)
{
  Q_UNUSED(tag);
  return calloc(size, 1);
}
static inline void* Z_Malloc(size_t size)
{
  return calloc(size, 1);
}
static inline void* S_Malloc(size_t size)
{
  return malloc(size);
}
static inline char* CopyString(const char* str)
{
  return strdup(str);
}
static inline void Z_Free(void* ptr)
{
  free(ptr);
}

void     Hunk_Clear( void );
void     Hunk_ClearToMark( void );
void     Hunk_SetMark( void );
qboolean Hunk_CheckMark( void );

//void *Hunk_Alloc( int size );
// void *Hunk_Alloc( int size, ha_pref preference );
void   Hunk_ClearTempMemory( void );
void   *Hunk_AllocateTempMemory( int size );
void   Hunk_FreeTempMemory( void *buf );
int    Hunk_MemoryRemaining( void );
void   Hunk_SmallLog( void );
void   Hunk_Log( void );

void   Com_TouchMemory( void );

double Sys_DoubleTime( void );

// commandLine should not include the executable name (argv[0])
void   Com_Init( char *commandLine );
void   Com_Frame( void (*GetInput)( void ), void (*DoneInput)( void ) );
void   Com_Shutdown();

/*
==============================================================

CLIENT / SERVER SYSTEMS

==============================================================
*/

//
// client interface
//
void CL_InitKeyCommands( void );

// the keyboard binding interface must be setup before execing
// config files, but the rest of client startup will happen later

void     CL_Init( void );
void     CL_ClearStaticDownload( void );
void     CL_Disconnect( qboolean showMainMenu );
void     CL_SendDisconnect( void );
void     CL_Shutdown( void );
void     CL_Frame( int msec );
void     CL_KeyEvent( int key, qboolean down, unsigned time );

void     CL_CharEvent( int c );

// char events are for field typing, not game control

void CL_MouseEvent( int dx, int dy, int time );

void CL_JoystickEvent( int axis, int value, int time );

void CL_PacketEvent( netadr_t from, msg_t *msg );

void CL_ConsolePrint( std::string text );

void CL_MapLoading( void );

// do a screen update before starting to load a map
// when the server is going to load a new map, the entire hunk
// will be cleared, so the client must shutdown cgame, ui, and
// the renderer

void CL_ForwardCommandToServer( const char *string );

// adds the current command line as a clc_clientCommand to the client message.
// things like godmode, noclip, etc, are commands directed to the server,
// so when they are typed in at the console, they will need to be forwarded.

void CL_ShutdownAll( void );

// shutdown all the client stuff

void CL_FlushMemory( void );

// dump all memory on an error

void CL_StartHunkUsers( void );

// start all the client stuff using the hunk

void Key_KeynameCompletion( void ( *callback )( const char *s ) );

// for keyname autocompletion

void Key_WriteBindings( fileHandle_t f );

// for writing the config files

void S_ClearSoundBuffer( void );

// AVI files have the start of pixel lines 4 byte-aligned
#define AVI_LINE_PADDING 4

//
// server interface
//
void     SV_Init( void );
void     SV_Shutdown( const char *finalmsg );
void     SV_Frame( int msec );
void     SV_PacketEvent( netadr_t from, msg_t *msg );
int      SV_FrameMsec( void );

/*
==============================================================

NON-PORTABLE SYSTEM SERVICES

==============================================================
*/

typedef enum
{
  AXIS_SIDE,
  AXIS_FORWARD,
  AXIS_UP,
  AXIS_ROLL,
  AXIS_YAW,
  AXIS_PITCH,
  MAX_JOYSTICK_AXIS
} joystickAxis_t;

typedef enum
{
  // bk001129 - make sure SE_NONE is zero
  SE_NONE = 0, // evTime is still valid
  SE_KEY, // evValue is a key code, evValue2 is the down flag
  SE_CHAR, // evValue is an ascii char
  SE_MOUSE, // evValue and evValue2 are relative, signed x / y moves
  SE_JOYSTICK_AXIS, // evValue is an axis number and evValue2 is the current state (-127 to 127)
  SE_CONSOLE, // evPtr is a char*
  SE_PACKET // evPtr is a netadr_t followed by data bytes to evPtrLength
} sysEventType_t;

typedef struct
{
    int            evTime;
    sysEventType_t evType;
    int            evValue, evValue2;
    int            evPtrLength; // bytes of data pointed to by evPtr, for journaling
    void           *evPtr; // this must be manually freed if not NULL
} sysEvent_t;

void       Com_QueueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr );
int        Com_EventLoop( void );

void       Sys_Init( void );
qboolean   Sys_IsNumLockDown( void );

void           *QDECL Sys_LoadDll( const char *name, intptr_t ( QDECL  * *entryPoint )( int, ... ),
                                   intptr_t ( QDECL *systemcalls )( intptr_t, ... ) );

void                  Sys_UnloadDll( void *dllHandle );

void                  *Sys_LoadFunction( void *dllHandle, const char *functionName );

const char            *Sys_GetCurrentUser( void );
int                   Sys_GetPID( void );

void QDECL NORETURN   Sys_Error( const char *error, ... ) PRINTF_LIKE(1);
void NORETURN         Sys_Quit( void );
char                  *Sys_GetClipboardData( clipboard_t clip );  // note that this isn't journaled...

void                  Sys_Print( const char *msg );

// Sys_Milliseconds should only be used for profiling purposes,
// any game related timing information should come from event timestamps
int           Sys_Milliseconds( void );

qboolean      Sys_RandomBytes( byte *string, int len );

// the system console is shown when a dedicated server is running
void          Sys_DisplaySystemConsole( qboolean show );

int           Sys_GetProcessorFeatures( void );

void          Sys_SetErrorText( const char *text );

void          Sys_SendPacket( int length, const void *data, netadr_t to );
qboolean      Sys_GetPacket( netadr_t *net_from, msg_t *net_message );

qboolean      Sys_StringToAdr( const char *s, netadr_t *a, netadrtype_t family );

//Does NOT parse port numbers, only base addresses.

qboolean Sys_IsLANAddress( netadr_t adr );
void     Sys_ShowIP( void );

FILE     *Sys_FOpen( const char *ospath, const char *mode );
qboolean Sys_Mkdir( const char *path );
FILE     *Sys_Mkfifo( const char *ospath );
char     *Sys_Cwd( void );
char     *Sys_DefaultBasePath( void );

void     Sys_FChmod( FILE *f, int mode );
void     Sys_Chmod( const char *ospath, int mode );

#ifdef MACOS_X
char     *Sys_DefaultAppPath( void );
#endif

char *Sys_DefaultLibPath( void );

char         *Sys_DefaultHomePath( void );
char         *Sys_Dirname( char *path );
char         *Sys_Basename( char *path );
char         *Sys_ConsoleInput( void );

void         Sys_Sleep( int msec );

typedef enum
{
  DR_YES = 0,
  DR_NO = 1,
  DR_OK = 0,
  DR_CANCEL = 1
} dialogResult_t;

typedef enum
{
  DT_INFO,
  DT_WARNING,
  DT_ERROR,
  DT_YES_NO,
  DT_OK_CANCEL
} dialogType_t;

dialogResult_t Sys_Dialog( dialogType_t type, const char *message, const char *title );

void           Sys_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr );

/* This is based on the Adaptive Huffman algorithm described in Sayood's Data
 * Compression book.  The ranks are not actually stored, but implicitly defined
 * by the location of a node within a doubly-linked list */

#define NYT           HMAX /* NYT = Not Yet Transmitted */
#define INTERNAL_NODE ( HMAX + 1 )

typedef struct nodetype
{
    struct nodetype *left, *right, *parent; /* tree structure */

    struct nodetype *next, *prev; /* doubly-linked list */

    struct nodetype **head; /* highest ranked node in block */

    int             weight;
    int             symbol;
} node_t;

#define HMAX 256 /* Maximum symbol */

typedef struct
{
    int    blocNode;
    int    blocPtrs;

    node_t *tree;
    node_t *lhead;
    node_t *ltail;
    node_t *loc[ HMAX + 1 ];
    node_t **freelist;

    node_t nodeList[ 768 ];
    node_t *nodePtrs[ 768 ];
} huff_t;

typedef struct
{
    huff_t compressor;
    huff_t decompressor;
} huffman_t;

void             Huff_Compress( msg_t *buf, int offset );
void             Huff_Decompress( msg_t *buf, int offset );
void             Huff_Init( huffman_t *huff );
void             Huff_addRef( huff_t *huff, byte ch );
int              Huff_Receive( node_t *node, int *ch, byte *fin );
void             Huff_transmit( huff_t *huff, int ch, byte *fout );
void             Huff_offsetReceive( node_t *node, int *ch, byte *fin, int *offset );
void             Huff_offsetTransmit( huff_t *huff, int ch, byte *fout, int *offset );
void             Huff_putBit( int bit, byte *fout, int *offset );
int              Huff_getBit( byte *fout, int *offset );

// don't use if you don't know what you're doing.
int              Huff_getBloc( void );
void             Huff_setBloc( int _bloc );

extern huffman_t clientHuffTables;

#define SV_ENCODE_START 4
#define SV_DECODE_START 12
#define CL_ENCODE_START 12
#define CL_DECODE_START 4

int  Parse_AddGlobalDefine( const char *string );
int  Parse_LoadSourceHandle( const char *filename );
int  Parse_FreeSourceHandle( int handle );
int  Parse_ReadTokenHandle( int handle, pc_token_t *pc_token );
int  Parse_SourceFileAndLine( int handle, char *filename, int *line );

void Com_RandomBytes( byte *string, int len );

#define _(x) Trans_Gettext(x)
#define C_(x, y) Trans_Pgettext(x, y)
#define N_(x) (x)
#define P_(x, y, c) Trans_GettextPlural(x, y, c)

void Trans_Init( void );
void Trans_LoadDefaultLanguage( void );
const char* Trans_Gettext( const char *msgid ) PRINTF_TRANSLATE_ARG(1);
const char* Trans_Pgettext( const char *ctxt, const char *msgid ) PRINTF_TRANSLATE_ARG(2);
const char* Trans_GettextPlural( const char *msgid, const char *msgid_plural, int num ) PRINTF_TRANSLATE_ARG(1) PRINTF_TRANSLATE_ARG(2);
const char* Trans_GettextGame( const char *msgid ) PRINTF_TRANSLATE_ARG(1);
const char* Trans_PgettextGame( const char *ctxt, const char *msgid ) PRINTF_TRANSLATE_ARG(2);
const char* Trans_GettextGamePlural( const char *msgid, const char *msgid_plural, int num ) PRINTF_TRANSLATE_ARG(1) PRINTF_TRANSLATE_ARG(2);
#endif // QCOMMON_H_
