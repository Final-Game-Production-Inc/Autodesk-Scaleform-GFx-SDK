/*---------------------------------------------------------------------------*
  Project:  Horizon
  File:     socket_Berkeley.h

  Copyright (C)2010 Nintendo Co., Ltd.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Rev: 35860 $
 *---------------------------------------------------------------------------*/

#ifndef NN_SOCKET_SOCKET_BERKLEY_H_
#define NN_SOCKET_SOCKET_BERKLEY_H_

#include <nn/socket/socket_Types.h>
#include <nn/socket/socket_Const.h>
#include <nn/socket/socket_IpcWrapper.h>
#include <nn/socket/socket_Result.h>

/*
    Uses IPC to mask the differences in return values and arguments
 */

namespace nn {
namespace socket {
    /* Please see man pages for details 
      

      
     */

    /* Please see man pages for details 
      

                    
                    
      
      
                                
      
      
                                
                                

      
      
      
      
      
      
      
      
      
      

      
    */
    inline s32 Socket(s32 af, s32 type, s32 protocol)
    {
        s32 rval = 0;
        Result result = detail::Socket(&rval, af, type, protocol);
        NN_SOCKET_RETURN_IF_FAILED(result);
        return rval;
    }

    /* Please see man pages for details 
      
                    
      
      
                                
      
      
      
      
                                
      
      
      
      
      

      
    */
    inline s32 Listen(s32 s, s32 backlog)
    {
        s32 rval = 0;
        Result result = detail::Listen(&rval, s, backlog);
        NN_SOCKET_RETURN_IF_FAILED(result);
        return rval;
    }

    /* Please see man pages for details 
      

                    
                    
                    
                    

                    
                    
 
                    
                    
                    
      
                                
                                
                                
                                
      
                                
      

      
      
      
      
      
                                
                                
      
      
      
      
      
      
      
    */
    inline s32 Accept(s32 s, SockAddrIn* sockAddr)
    {
        s32 rval = 0;
        Result result = detail::Accept(&rval, s, reinterpret_cast<u8*>(sockAddr), sizeof(SockAddrIn));
        NN_SOCKET_RETURN_IF_FAILED(result);
        return rval;
    }

    /* Please see man pages for details 
      
      
                    
                    
                    
                    
                    
                    
      
      

      
      
      
      
      
                                
                                
      
      
      
      
    */
    inline s32 Bind(s32 s, const SockAddrIn* sockAddr)
    {
        s32 rval = 0;
        Result result = detail::Bind(&rval, s, reinterpret_cast<const u8*>(sockAddr), sizeof(SockAddrIn));
        NN_SOCKET_RETURN_IF_FAILED(result);
        return rval;
    }

    /* Please see man pages for details 
      

                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
      
                    
      
      
      
      
      
      
      
      
      
                                
                                
      
      
                                

      
      
      
      
      
      
      
      
    */
    inline s32 Connect(s32 s, const SockAddrIn* sockAddr)
    {
        s32 rval = 0;
        Result result = detail::Connect(&rval, s, reinterpret_cast<const u8*>(sockAddr), sizeof(SockAddrIn));
        NN_SOCKET_RETURN_IF_FAILED(result);
        return rval;
    }

    /* Please see man pages for details 
      
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
      
      
      
                                
      
      
                                
      
      
                                
                                
                                
      
                    
      
                                
      
                                
      
                                
                                
                                
                                
      
                                
                                
                                
                                
      
      
      
      
      
                                
      

      
                                
      
      
    */
    inline s32 RecvFrom(s32 s, void* buf, s32 len, s32 flags, SockAddrIn* sockFrom)
    {
        s32 rval;
        Result result;
        if (len <= detail::COPY_OR_MAP_THRESHOLD)
        {
            result = detail::RecvFromSmall(&rval, s, reinterpret_cast<u8*>(buf), len, flags, reinterpret_cast<u8*>(sockFrom), sizeof(SockAddrIn));
        }
        else
        {
            result = detail::RecvFrom(&rval, s, reinterpret_cast<u8*>(buf), len, flags, reinterpret_cast<u8*>(sockFrom), sizeof(SockAddrIn));
        }
        NN_SOCKET_RETURN_IF_FAILED(result);
        return rval;
    }

    /* Please see man pages for details 
      
                    
                    
                    

      
      
      
      
 
      
    */
    inline s32 Recv(s32 s, void* buf, s32 len, s32 flags)
    {
        return RecvFrom(s, buf, len, flags, NULL);
    }

    /* Please see man pages for details 
      
                    
                    
                    

      
      
      

      
    */
    inline s32 Read(s32 s, void* buf, s32 len)
    {
        return RecvFrom(s, buf, len, 0, NULL);
    }

    /* Please see man pages for details 
      
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
      
      
      
      
      
                                
      
      
      
                                
      
      
                                
                                
                                
                                
      
      
      
      
                                
      
      
      
      
      
      
      
      
                                
      

      
    */
    inline s32 SendTo(s32 s, const void* buf, s32 len, s32 flags, const SockAddrIn* sockTo)
    {
        s32 rval;
        Result result;
        if (len <= detail::COPY_OR_MAP_THRESHOLD)
        {
            result = detail::SendToSmall(&rval, s,
                                         reinterpret_cast<const u8*>(buf), len,
                                         flags,
                                         reinterpret_cast<const u8*>(sockTo), sizeof(SockAddrIn));
        }
        else
        {
            result = detail::SendTo(&rval, s,
                                    reinterpret_cast<const u8*>(buf), len,
                                    flags,
                                    reinterpret_cast<const u8*>(sockTo), sizeof(SockAddrIn));
        }
        NN_SOCKET_RETURN_IF_FAILED(result);
        return rval;
    }

    inline s32 SendToMulti(s32 s, const void* buf, s32 len, s32 flags, const SockAddrIn* sockTo, s32 saCount)
    {
        s32 rval;
        Result result;
        result = detail::SendToSmallMulti(&rval, s,
                                          reinterpret_cast<const u8*>(buf), len,
                                          flags,
                                          reinterpret_cast<const u8*>(sockTo), sizeof(SockAddrIn), sizeof(SockAddrIn) * saCount);
        NN_SOCKET_RETURN_IF_FAILED(result);
        return rval;
    }
    /* Please see man pages for details 
      
                    
                    
                    

      
      
      
      

      
    */
    inline s32 Send(s32 s, const void* buf, s32 len, s32 flags)
    {
        return SendTo(s, buf, len, flags, NULL);
    }

    /* Please see man pages for details 
      
                    
                    
                    
                    
      
      
      

      
    */
    inline s32 Write(s32 s, const void* buf, s32 len)
    {
        return SendTo(s, buf, len, 0, NULL);
    }

    /* Please see man pages for details 
      
      
                    
                    
                    
                    
                    
                    
                    
                    
                    
      
      
      
      
      
      
      
      
      
    */
    inline s32 Close(s32 s)
    {
        s32 rval = 0;
        Result result = detail::Close(&rval, s);
        NN_SOCKET_RETURN_IF_FAILED(result);
        return rval;
    }

    /* Please see man pages for details 
      
                    
                    
      
      
                                
                                
                                

      
      
      
                                
      
      
      
      
      
    */
    inline s32 Shutdown(s32 s, s32 how)
    {
        s32 rval = 0;
        Result result = detail::Shutdown(&rval, s, how);
        NN_SOCKET_RETURN_IF_FAILED(result);
        return rval;
    }

    /* Please see man pages for details 
      
                    
      
      
      
      
      
      
      
      
      
                                
      
      
      
      
      
    */
    inline s32 GetSockOpt(s32 s, s32 level, int optname, void* optval, int* optlen)
    {
        s32 rval = 0;
        Result result = detail::GetSockOpt(&rval, s, level, optname, reinterpret_cast<u8*>(optval), optlen);
        NN_SOCKET_RETURN_IF_FAILED(result);
        return rval;
    }

    /* Please see man pages for details 
      
                    
      
      
      
      
      
      
      
      
      
                                
                                
      
      
      
      
      
    */
    inline s32 SetSockOpt(s32 s, s32 level, s32 optname, const void* optval, s32 optlen)
    {
        s32 rval = 0;
        Result result = detail::SetSockOpt(&rval, s, level, optname, reinterpret_cast<const u8*>(optval), optlen);
        NN_SOCKET_RETURN_IF_FAILED(result);
        return rval;
    }

    /* Please see man pages for details 
      
                    
      
      
                                
                                
      
                                
                                
                                
                                
                                
                                    
                                    
                    
      
      
      
                                
      
      
      

      
    */
    inline s32 Fcntl( s32 s, s32 cmd, s32 val )
    {
        s32 rval = 0;
        Result result = detail::Fcntl(&rval, s, cmd, val);
        NN_SOCKET_RETURN_IF_FAILED(result);
        return rval;
    }

    /* Please see man pages for details 
      
      
                    
                    
                    
                    
                    
      
      
      
                                
                                
                                

      
      
      
                                
                                
      
      

      
    */
    inline s32 Poll( PollFd fds[], u32 nfds, s32 timeout )
    {
        s32 rval = 0;
        Result result = detail::Poll(&rval, fds, nfds, timeout);
        NN_SOCKET_RETURN_IF_FAILED(result);
        return rval;
    }

    /* Please see man pages for details 
      

                    
                    
      
      
      
                                
                                
                                
                                

      
                                
                                

      
      
      
      

      
    */
    inline s32 SockAtMark( s32 s )
    {
        s32 rval = 0;
        Result result = detail::SockAtMark(&rval, s);
        NN_SOCKET_RETURN_IF_FAILED(result);
        return rval;
    }

    /* Please see man pages for details 
      
                    
      
      
      
    */
    inline u32 GetHostId( void )
    {
        u32 rval = 0;
        detail::GetHostId(&rval);
        return rval;
    }

    /* Please see man pages for details 
      

                    
                    
                    
                    
                    
      
      
      
      
      
      
      
      

      
    */
    inline s32 GetSockName( s32 s, SockAddrIn* sockAddr )
    {
        s32 rval = 0;
        Result result = detail::GetSockName(&rval, s, reinterpret_cast<u8*>(sockAddr), sizeof(SockAddrIn));
        NN_SOCKET_RETURN_IF_FAILED(result);
        return rval;
    }

    /* Please see man pages for details 
      

                    
                    
      
      
      
      
      
      
      
      

      
    */
    inline s32 GetPeerName( s32 s, SockAddrIn* sockAddr )
    {
        s32 rval = 0;
        Result result = detail::GetPeerName(&rval, s, reinterpret_cast<u8*>(sockAddr), sizeof(SockAddrIn));
        NN_SOCKET_RETURN_IF_FAILED(result);
        return rval;
    }

    inline s32 GetNetworkOpt( s32 level, s32 optname, void* optval, s32* optlen )
    {
        s32 rval = 0;
        Result result = detail::GetNetworkOpt(&rval, level, optname, reinterpret_cast<u8*>(optval), optlen);
        NN_SOCKET_RETURN_IF_FAILED(result);
        return rval;
    }

    inline s32 GetResolverInfo( nn::socket::DnsServerInfo* pDnsServerInfo )
    {
        s32 rval = 0;
        Result result = detail::GetResolverInfo(&rval, pDnsServerInfo);
        NN_SOCKET_RETURN_IF_FAILED(result);
        return rval;
    }

    // DnsUserClient
    /* Please see man pages for details 
      
      
                    
                    
                    
                    
      
                                
      
      
                    
                    
                    

      
    */
    HostEnt* GetHostByName(const char8* name);

    /* Please see man pages for details 
      
      
                    
                    
      
      
                                
      
                                
                                
      
                    
                    
                    
      
    */
    HostEnt* GetHostByAddr(const void* addr, s32 len, s32 type);

    /* Please see man pages for details 
      
      
                    
                    
                    
                    
                    
      
                                
      
                                
      
      
      
      
      
                                
                                
                                
                                
                                
                                
                                
      
      

      
    */
    s32 GetAddrInfo(const char8* nodeName, const char8* servName, const AddrInfo* hints, AddrInfo** res);

    /* Please see man pages for details 
      
                    
                    
      
                                
                                
      
      
    */
    void FreeAddrInfo(AddrInfo* head);

    /* Please see man pages for details 
      

                    
                    
      
      
      
      
      
      
                                
                                  
                                
                                  
                                
                                  
                                
                                  

      
      
      

      
    */
    s32 GetNameInfo(const void* sa, char8* node, s32 nodeLen, char8* service, s32 serviceLen, s32 flags);

    /* Please see man pages for details 
      

      
      
     */

    // InetUtils
    /* Please see man pages for details 
      

                    
                    
      
                                
      
      
      
      

      
    */
    s32 InetAtoN(const char* cp, InAddr* inp);

    /* Please see man pages for details 
      

                    
                    
                    
                    
                    
      
      
      
                    

      
    */
    char* InetNtoA(InAddr in);

    /* Please see man pages for details 
      

                    
                    
      
      
                                
                                
                                
      
                                
                                
      
      
      

      
    */
    s32 InetPtoN(int af, const char* src, void* dst);

    /* Please see man pages for details 
      

                    
                    
      
                                
                                
      
                                

                                
      
                                

      
                                

      
                    
                    

      
    */
    const char* InetNtoP(int af, const void* src, char* dst, unsigned len);

    // byteorder
    /* Please see man pages for details 
      
                    
                    
      
      
      

      
    */
    inline bit32 HtoNl(bit32 v)
    {
        return NN_SOCKET_HtoNl(v);
    }
    
    /* Please see man pages for details 
      
                    
      
      
      

      
    */
    inline bit32 NtoHl(bit32 v)
    {
        return NN_SOCKET_NtoHl(v);
    }
    /* Please see man pages for details 
      
                    
      
      
      

      
    */
    inline bit16 HtoNs(bit16 v)
    {
        return NN_SOCKET_HtoNs(v);
    }
    /* Please see man pages for details 
      
                    
      
      
      

      
    */
    inline bit16 NtoHs(bit16 v)
    {
        return NN_SOCKET_NtoHs(v);
    }
}
}

#endif  // ifndef NN_SOCKET_SOCKET_BERKLEY_H_
