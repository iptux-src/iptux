/*
 *  Copyright (C) 2006 Takeharu KATO
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if !defined(IPMSG_H)
#define IPMSG_H
#define IPMSG_VERSION                   0x001
#define IPMSG_PORT                      0x979

/*  command  */
#define IPMSG_NOOPERATION               0x00000000UL

#define IPMSG_BR_ENTRY          0x00000001UL
#define IPMSG_BR_EXIT                   0x00000002UL
#define IPMSG_ANSENTRY          0x00000003UL
#define IPMSG_BR_ABSENCE                0x00000004UL

#define IPMSG_BR_ISGETLIST              0x00000010UL
#define IPMSG_OKGETLIST         0x00000011UL
#define IPMSG_GETLIST                   0x00000012UL
#define IPMSG_ANSLIST                   0x00000013UL
#define IPMSG_BR_ISGETLIST2             0x00000018UL

#define IPMSG_SENDMSG           0x00000020UL
#define IPMSG_RECVMSG           0x00000021UL
#define IPMSG_READMSG           0x00000030UL
#define IPMSG_DELMSG                    0x00000031UL
#define IPMSG_ANSREADMSG                0x00000032UL

#define IPMSG_GETINFO                   0x00000040UL
#define IPMSG_SENDINFO          0x00000041UL

#define IPMSG_GETABSENCEINFO    0x00000050UL
#define IPMSG_SENDABSENCEINFO   0x00000051UL

#define IPMSG_GETFILEDATA               0x00000060UL
#define IPMSG_RELEASEFILES              0x00000061UL
#define IPMSG_GETDIRFILES               0x00000062UL

#define IPMSG_GETPUBKEY         0x00000072UL
#define IPMSG_ANSPUBKEY         0x00000073UL

/*  option for all command  */
#define IPMSG_ABSENCEOPT                0x00000100UL
#define IPMSG_SERVEROPT         0x00000200UL
#define IPMSG_DIALUPOPT         0x00010000UL
#define IPMSG_FILEATTACHOPT             0x00200000UL
#define IPMSG_ENCRYPTOPT                0x00400000UL
#define IPMSG_UTF8OPT           0x00800000UL

/*  option for send command  */
#define IPMSG_SENDCHECKOPT              0x00000100UL
#define IPMSG_SECRETOPT         0x00000200UL
#define IPMSG_BROADCASTOPT              0x00000400UL
#define IPMSG_MULTICASTOPT              0x00000800UL
#define IPMSG_NOPOPUPOPT                0x00001000UL
#define IPMSG_AUTORETOPT                0x00002000UL
#define IPMSG_RETRYOPT          0x00004000UL
#define IPMSG_PASSWORDOPT               0x00008000UL
#define IPMSG_NOLOGOPT          0x00020000UL
#define IPMSG_NEWMUTIOPT                0x00040000UL
#define IPMSG_NOADDLISTOPT              0x00080000UL
#define IPMSG_READCHECKOPT              0x00100000UL
#define IPMSG_SECRETEXOPT               (IPMSG_READCHECKOPT|IPMSG_SECRETOPT)

#define IPMSG_NO_REPLY_OPTS             (IPMSG_BROADCASTOPT|IPMSG_AUTORETOPT)

/* encryption flags for encrypt command */
#define IPMSG_RSA_512                   0x00000001UL
#define IPMSG_RSA_1024          0x00000002UL
#define IPMSG_RSA_2048          0x00000004UL
#define IPMSG_RC2_40                    0x00001000UL
#define IPMSG_RC2_128                   0x00004000UL
#define IPMSG_RC2_256                   0x00008000UL
#define IPMSG_BLOWFISH_128              0x00020000UL
#define IPMSG_BLOWFISH_256              0x00040000UL
#define IPMSG_AES_128                   0x00100000UL
#define IPMSG_AES_192                   0x00200000UL
#define IPMSG_AES_256                   0x00400000UL
#define IPMSG_SIGN_STAMPOPT             0x01000000UL
#define IPMSG_SIGN_MD5          0x10000000UL
#define IPMSG_SIGN_SHA1         0x20000000UL

/* compatibilty for Win beta version */
#define IPMSG_RC2_40OLD         0x00000010UL    // for beta1-4 only
#define IPMSG_RC2_128OLD                0x00000040UL    // for beta1-4 only
#define IPMSG_BLOWFISH_128OLD   0x00000400UL    // for beta1-4 only
#define IPMSG_RC2_40ALL         (IPMSG_RC2_40|IPMSG_RC2_40OLD)
#define IPMSG_RC2_128ALL                (IPMSG_RC2_128|IPMSG_RC2_128OLD)
#define IPMSG_BLOWFISH_128ALL   (IPMSG_BLOWFISH_128|IPMSG_BLOWFISH_128OLD)

/* file types for fileattach command */
#define IPMSG_FILE_REGULAR              0x00000001UL
#define IPMSG_FILE_DIR                  0x00000002UL
#define IPMSG_FILE_RETPARENT            0x00000003UL    // return parent directory
#define IPMSG_FILE_SYMLINK              0x00000004UL
#define IPMSG_FILE_CDEV         0x00000005UL    // for UNIX
#define IPMSG_FILE_BDEV         0x00000006UL    // for UNIX
#define IPMSG_FILE_FIFO         0x00000007UL    // for UNIX
#define IPMSG_FILE_RESFORK              0x00000010UL    // for Mac

/* file attribute options for fileattach command */
#define IPMSG_FILE_RONLYOPT             0x00000100UL
#define IPMSG_FILE_HIDDENOPT            0x00001000UL
#define IPMSG_FILE_EXHIDDENOPT  0x00002000UL    // for MacOS X
#define IPMSG_FILE_ARCHIVEOPT   0x00004000UL
#define IPMSG_FILE_SYSTEMOPT    0x00008000UL

/* extend attribute types for fileattach command */
#define IPMSG_FILE_UID                  0x00000001UL
#define IPMSG_FILE_USERNAME             0x00000002UL    // uid by string
#define IPMSG_FILE_GID                  0x00000003UL
#define IPMSG_FILE_GROUPNAME    0x00000004UL    // gid by string
#define IPMSG_FILE_PERM         0x00000010UL    // for UNIX
#define IPMSG_FILE_MAJORNO              0x00000011UL    // for UNIX devfile
#define IPMSG_FILE_MINORNO              0x00000012UL    // for UNIX devfile
#define IPMSG_FILE_CTIME                0x00000013UL    // for UNIX
#define IPMSG_FILE_MTIME                0x00000014UL
#define IPMSG_FILE_ATIME                0x00000015UL
#define IPMSG_FILE_CREATETIME   0x00000016UL
#define IPMSG_FILE_CREATOR              0x00000020UL    // for Mac
#define IPMSG_FILE_FILETYPE             0x00000021UL    // for Mac
#define IPMSG_FILE_FINDERINFO   0x00000022UL    // for Mac
#define IPMSG_FILE_ACL          0x00000030UL
#define IPMSG_FILE_ALIASFNAME   0x00000040UL    // alias fname
#define IPMSG_FILE_UNICODEFNAME         0x00000041UL    // UNICODE fname

#define FILELIST_SEPARATOR              '\a'
#define HOSTLIST_SEPARATOR              '\a'
#define HOSTLIST_DUMMY          "\b"
#define HLIST_ENTRY_SEPARATOR   ':'
#endif

///*    @(#)Copyright (C) Jally 2008   iptux.h    Version 0.4 *///
#ifdef __IP_TUX__

/* macro */
#define GET_MODE(command)               (command & 0x000000ffUL)
#define GET_OPT(command)                (command & 0xffffff00UL)

/* header */
#define IPTUX_VERSION                   "1_iptux_0#5#2"
#define IPTUX_DEFAULT_PORT              IPMSG_PORT

/* command */
#define IPTUX_ASKSHARED         0x000000FFUL
#define IPTUX_SENDICON          0x000000FEUL
#define IPTUX_SENDSUBLAYER              0x000000FDUL
#define IPTUX_SENDSIGN          0x000000FCUL
#define IPTUX_SENDMSG           0x000000FBUL
/* option for IPTUX_SENDSUBLAYER */
#define IPTUX_PHOTOPICOPT               0x00000100UL
#define IPTUX_MSGPICOPT         0x00000200UL
/* option for IPMSG_SENDMSG */
#define IPTUX_SHAREDOPT         0x80000000UL
/* option for IPMSG_SENDMSG & IPTUX_ASKSHARED */
#define IPTUX_PASSWDOPT         0x40000000UL
/* option for IPTUX_SENDMSG */
#define IPTUX_REGULAROPT                0x00000100UL
#define IPTUX_SEGMENTOPT                0x00000200UL
#define IPTUX_GROUPOPT          0x00000300UL
#define IPTUX_BROADCASTOPT              0x00000400UL

/* data */
#define MAX_SOCKLEN                     8192
#define MAX_UDPLEN                      8192
#define MAX_BUFLEN                      1024
#define MAX_PATHLEN                     1024
#define MAX_SHAREDFILE          10000
#define MAX_ICONSIZE                    30
#define MAX_PREVIEWSIZE         150
#define MAX_PHOTOSIZE           300
#define MAX_RETRYTIMES          4

#endif
