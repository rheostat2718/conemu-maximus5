
/*
Copyright (c) 2011 Maximus5
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#ifndef PIPEBUFSIZE
#define PIPEBUFSIZE 4096
#endif

// struct - ��� ����, ����� �������� ������ ������� calloc. "new" �� �����������, �.�. Crt ����� ���� �� ��������������� (ConEmuHk).

// ����� ������������, ��� ������ DWORD � (T*) ��� ������ ������������� ����� � ������!
template <class T>
struct PipeServer
{
	protected:
		enum PipeState
		{
			CONNECTING_STATE = 0,
			READING_STATE = 1,
			WRITING_STATE = 2,
			ERROR_STATE = 3,
		};
		struct PipeInst
		{
			// Common
			HANDLE hPipeInst; // Pipe Handle
			
			HANDLE hEvent; // Overlapped event
			OVERLAPPED oOverlap;
			BOOL fPendingIO;
			PipeState dwState;
			
			wchar_t sErrorMsg[128];
			
			// Request (from client)
			DWORD cbReadSize, cbMaxReadSize;
			T* ptrRequest;
			T* AllocRequestBuf(DWORD anSize)
			{
				if (ptrRequest && (cbMaxReadSize >= anSize))
					return ptrRequest;

				if (ptrRequest)
					free(ptrRequest);

				ptrRequest = (T*)malloc(anSize);
				cbMaxReadSize = anSize;
				return ptrRequest;
			};
			// Answer (to client)
			DWORD cbReplySize, cbMaxReplySize;
			T* ptrReply; // !!! ������ ���������� � mfn_PipeServerCommand
			//T* AllocReplyBuf(DWORD anSize)
			//{
			//	if (ptrReply && (cbMaxReplySize >= anSize))
			//		return ptrReply;
			//	if (ptrReply)
			//		free(ptrReply);
			//	ptrReply = (T*)malloc(anSize);
			//	cbMaxReplySize = anSize;
			//	return ptrReply;
			//};
			//// Event descriptors
			//HANDLE hTermEvent;
			//HANDLE hServerSemaphore;

			// Pointer to object
			PipeServer *pServer;

			// Server thread.
			HANDLE hThread;
			DWORD nThreadId;
		};
	protected:
		BOOL mb_Initialized;
		BOOL mb_Overlapped;
		BOOL mb_LoopCommands;
		wchar_t ms_PipeName[MAX_PATH];
		int MaxCount;
		PipeInst m_Pipes[3];
		PipeInst *mp_ActivePipe;
		
		LPSECURITY_ATTRIBUTES mlp_Sec;
		
		// *** Callback function and it's LParam ***
		
		// �������� �������� �������. ���������, ������� ���������.
		// ��� ���������� ����� - ������ �������� ������, ������ ����� ������������ �� �� ������ (*pcbMaxReplySize)
		typedef BOOL (WINAPI* PipeServerCommand_t)(T* pCmd, T* &ppReply, DWORD &pcbReplySize, DWORD &pcbMaxReplySize, LPARAM lParam, HANDLE hPipe);
		PipeServerCommand_t mfn_PipeServerCommand;
		
		// ���������� ����� ����������� ������� � ���������� �����, �� ����� PipeServerRead � PipeServerCommand_t
		// (������� ����� ���� ����� �� ����� �����������)
		typedef BOOL (WINAPI* PipeServerConnected_t)(LPARAM lParam, HANDLE hNamedPipe);
		PipeServerConnected_t mfn_PipeServerConnected;
		// �����������, ��� ������ ��������� (��� ��� ���������). ���������� ������ ���� ��� mfn_PipeServerConnected
		PipeServerConnected_t mfn_PipeServerDisconnected;
		
		// ���������� ����� ����, ��� ������ Pipe Instance
		typedef BOOL (WINAPI* PipeServerReady_t)(LPARAM lParam);
		PipeServerReady_t mfn_PipeServerReady;
		
		// ���������� ������, ���������� ��� ���������
		typedef void (WINAPI* PipeServerFree_t)(T* pReply, LPARAM lParam);
		PipeServerFree_t mfn_PipeServerFree;
		
		// Params
		BOOL mb_ReadyCalled;
		LPARAM m_lParam; // ������ ��� StartPipeServer, ���������� � �������
	protected:
		// Event descriptors
		HANDLE mh_TermEvent;
		BOOL   mb_Terminate;
		HANDLE mh_ServerSemaphore;

		void DumpError(PipeInst *pPipe, LPCWSTR asFormat, BOOL abShow = TRUE)
		{
			wchar_t sTitle[128], sText[MAX_PATH*2];
			DWORD dwErr = GetLastError();
			msprintf(pPipe->sErrorMsg, countof(pPipe->sErrorMsg), asFormat/*i.e. L"ConnectNamedPipe failed with 0x%08X."*/, dwErr);

			if (abShow)
			{
				msprintf(sText, countof(sText), L"%s\n%s", pPipe->sErrorMsg, ms_PipeName);
				msprintf(sTitle, countof(sTitle), L"PipeServerError, PID=%u, TID=%u",
				                GetCurrentProcessId(), GetCurrentThreadId());
				GuiMessageBox(NULL, sText, sTitle, MB_ICONSTOP|MB_SYSTEMMODAL);
			}
			else
			{
				msprintf(sText, countof(sText), L"PipeServerError, PID=%u, TID=%u\n  %s\n  %s\n",
				                GetCurrentProcessId(), GetCurrentThreadId(),
				                pPipe->sErrorMsg, ms_PipeName);
				OutputDebugStringW(sText);
			}
		}

#if 0
		// Overlapped, ��������
		// ConnectToNewClient(HANDLE, LPOVERLAPPED)
		// This function is called to start an overlapped connect operation.
		// It returns TRUE if an operation is pending or FALSE if the
		// connection has been completed.
		BOOL ConnectToNewClient(PipeInst* pPipe)
		{
			BOOL fConnected;
			DWORD dwErr = 0;
			pPipe->fPendingIO = FALSE;
			pPipe->cbReadSize = 0;
			pPipe->dwState = ERROR_STATE;
			// Start an overlapped connection for this pipe instance.
			fConnected = ConnectNamedPipe(pPipe->hPipeInst, &pPipe->oOverlap);
			dwErr = GetLastError();

			// Overlapped ConnectNamedPipe should return zero.
			if (fConnected)
			{
				DumpError(pPipe, L"ConnectNamedPipe failed with 0x%08X", FALSE);

				if (!mb_Terminate)
					Sleep(100);
			}
			else
			{
				switch(dwErr)
				{
						// The overlapped connection in progress.
					case ERROR_IO_PENDING:
						pPipe->fPendingIO = TRUE;
						pPipe->dwState = CONNECTING_STATE;
						break;
						// Client is already connected, so signal an event.
					case ERROR_PIPE_CONNECTED:
						pPipe->dwState = READING_STATE;
						SetEvent(pPipe->oOverlap.hEvent);
						break;
						// If an error occurs during the connect operation...
					default:
						DumpError(pPipe, L"ConnectNamedPipe failed with 0x%08X", FALSE);

						if (!mb_Terminate)
							Sleep(100);
				}
			}

			return pPipe->fPendingIO;
		}
#endif

#if 0
		// Overlapped, ��������
		int InitPipes()
		{
			if (mb_Initialized)
			{
				_ASSERTE(mb_Initialized==FALSE);
				return -1000;
			}

			memset(m_Pipes, 0, sizeof(m_Pipes));

			// The initial loop creates several instances of a named pipe
			// along with an event object for each instance.  An
			// overlapped ConnectNamedPipe operation is started for
			// each instance.

			for (int i = 0; i < MaxCount; i++)
			{
				m_Pipes[i].pServer = this;

				// Create an event object for this instance.

				if (m_Pipes[i].hEvent == NULL)
					m_Pipes[i].hEvent = CreateEvent(
					                        NULL,    // default security attribute
					                        TRUE,    // manual-reset(!) event
					                        TRUE,    // initial state = signaled
					                        NULL);   // unnamed event object

				if (m_Pipes[i].hEvent == NULL)
				{
					DumpError(pPipe, L"CreateEvent failed with 0x%08X");
					return -(100+i);
				}

				m_Pipes[i].oOverlap.hEvent = m_Pipes[i].hEvent;
				_ASSERTE(LocalSecurity()!=NULL);
				m_Pipes[i].hPipeInst = CreateNamedPipeW(
				                           ms_PipeName,            // pipe name
				                           PIPE_ACCESS_DUPLEX |    // read/write access
				                           FILE_FLAG_OVERLAPPED,   // overlapped mode
				                           PIPE_TYPE_MESSAGE |     // message-type pipe
				                           PIPE_READMODE_MESSAGE | // message-read mode
				                           PIPE_WAIT,              // blocking mode
				                           MaxCount,               // number of instances
				                           sizeof(T),              // output buffer size
				                           sizeof(T),              // input buffer size
				                           0,                      // client time-out
				                           LocalSecurity());     // default security attributes

				if (m_Pipes[i].hPipeInst == INVALID_HANDLE_VALUE)
				{
					DumpError(pPipe, L"CreateNamedPipe failed with 0x%08X");
					return -(200+i);
				}

				// Call the subroutine to connect to the new client and update state
				ConnectToNewClient(m_Pipes+i);
			}

			mb_Initialized = TRUE;
			return 0;
		}
#endif

#if 0
		// Overlapped, ��������
		
		// DisconnectAndReconnect(PipeInst*)
		// This function is called when an error occurs or when the client
		// closes its handle to the pipe. Disconnect from this client, then
		// call ConnectNamedPipe to wait for another client to connect.
		void DisconnectAndReconnect(PipeInst* pPipe)
		{
			pPipe->cbReadSize = 0;

			// Disconnect the pipe instance
			if (!DisconnectNamedPipe(pPipe->hPipeInst))
			{
				DumpError(pPipe, L"DisconnectNamedPipe failed with 0x%08X", FALSE);
			}

			pPipe->dwState = ERROR_STATE;
			// Call a subroutine to connect to the new client and update state
			ConnectToNewClient(pPipe);
		}
#endif

#if 0
		int GetAnswerToRequest(PipeInst* pPipe)
		{
			if (!mfn_PipeServerCommand(pPipe->ptrRequest, &pPipe->ptrReply, &pPipe->cbReplySize, &pPipe->cbMaxReplySize, m_lParam))
			{
				if ((pPipe->ptrReply == NULL) || (pPipe->cbMaxReplySize < sizeof(DWORD)))
				{
					if (pPipe->ptrReply) free(pPipe->ptrReply);

					pPipe->cbMaxReplySize = (DWORD)max(sizeof(DWORD),sizeof(T));
					pPipe->ptrReply = (T*)malloc(pPipe->cbMaxReplySize);
					*((DWORD*)pPipe->ptrReply) = 0;
					pPipe->cbReplySize = sizeof(DWORD);
				}
			}

			return pPipe->cbReplySize;
		}
#endif

#if 0
		// Overlapped, ��������
		
		int PipeServerWork(PipeInst* pPipe)
		{
			DWORD i, dwWait, cbRet, cbRead, dwErr;
			int nToRead;
			BOOL fSuccess;
			HANDLE hEvents[2] = {pPipe->hEvent, mh_TermEvent};
			_ASSERTE(hEvents[1] != NULL);
			pPipe->cbReadSize = 0;

			while (!mb_Terminate)
			{
				// Wait for the event object to be signaled, indicating
				// completion of an overlapped read, write, or
				// connect operation.
				dwWait = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

				if (mb_Terminate)
				{
					// ������ ��� ����������� ���������� �������
					return -1;
				}

				// Get the result if the operation was pending.
				if (pPipe->fPendingIO)
				{
					fSuccess = GetOverlappedResult(
					               pPipe->hPipeInst, // handle to pipe
					               &pPipe->oOverlap, // OVERLAPPED structure
					               &cbRet,           // bytes transferred
					               FALSE);           // do not wait

					switch(pPipe->dwState)
					{
							// Pending connect operation
						case CONNECTING_STATE:

							if (!fSuccess)
							{
								DumpError(pPipe, L"Error while connecting 0x%08X", FALSE);
								DisconnectAndReconnect(pPipe);
								continue;
							}

							pPipe->dwState = READING_STATE;
							pPipe->cbReadSize = 0;
							break;
							// Pending read operation
						case READING_STATE:

							if (!fSuccess || (cbRet == 0))
							{
								DisconnectAndReconnect(pPipe);
								continue;
							}

							pPipe->cbReadSize += cbRet;
							nToRead = (*((DWORD*)pPipe->ptrRequest)) - pPipe->cbReadSize;

							if (nToRead <= 0)
							{
								// ���� ������� _���_ ������
								pPipe->dwState = WRITING_STATE;
							}

							break;
							// Pending write operation
						case WRITING_STATE:
							// Write operation finished, reconnect
							TODO("��� ����� ���� �� �� ����������������, ���� ��� ������������� ������ �������������� �������");
							// ������������� ������� ����� �������������� �� ������.
							// ��������, �� �������� ��� Pipe ����������� �����
							// (������-ConEmuC, ������-ConEmu, �������� �� �����, ����� ��� ����� Detach/Attach)
							DisconnectAndReconnect(pPipe);
							continue;
						default:
						{
							// Invalid pipe state
							_ASSERTE(pPipe->dwState<=WRITING_STATE);
							pPipe->dwState = ERROR_STATE;
						}
					}
				}

				// The pipe state determines which operation to do next.

				// READING_STATE:
				// The pipe instance is connected to the client
				// and is ready to read a request from the client.
				if (pPipe->dwState == READING_STATE)
				{
					//if (pPipe->ptrRequest == NULL)
					//{
					//	pPipe->ptrRequest = (T*)malloc(sizeof(T));
					//	pPipe->cbMaxReadSize = sizeof(T);
					//}
					if (pPipe->cbReadSize == 0)
					{
						nToRead = sizeof(T);
					}
					else
					{
						nToRead = (*((DWORD*)pPipe->ptrRequest)) - pPipe->cbReadSize;
					}

					if (nToRead <= 0)
					{
						_ASSERTE(nToRead > 0);
						DisconnectAndReconnect(pPipe);
						continue;
					}

					// Allocate read buffer
					if ((nToRead + pPipe->cbReadSize) >= pPipe->cbMaxReadSize)
					{
						pPipe->cbMaxReadSize = (nToRead + pPipe->cbReadSize) + 0x100;
						T* ptrNew = (T*)malloc(pPipe->cbMaxReadSize);

						if (pPipe->ptrRequest)
						{
							if (pPipe->cbReadSize)
								memmove(ptrNew, pPipe->ptrRequest, pPipe->cbReadSize);

							free(pPipe->ptrRequest);
						}

						pPipe->ptrRequest = ptrNew;
					}

					// Quering next input block
					_ASSERTE(pPipe->cbMaxReadSize > pPipe->cbReadSize);
					fSuccess = ReadFile(
					               pPipe->hPipeInst,
					               ((LPBYTE)pPipe->ptrRequest) + pPipe->cbReadSize,
					               nToRead,
					               &cbRead,
					               &pPipe->oOverlap);

					// The read operation completed successfully (w/o pending).
					if (fSuccess && (cbRead != 0))
					{
						pPipe->fPendingIO = FALSE;
						pPipe->cbReadSize += cbRead;
						nToRead = (*((DWORD*)pPipe->ptrRequest)) - pPipe->cbReadSize;

						if (nToRead > 0)
							continue; // ��������� �������� ������

						pPipe->dwState = WRITING_STATE;
					}
					else
					{
						// The read operation is still pending.
						dwErr = GetLastError();

						if (!fSuccess && (dwErr == ERROR_IO_PENDING))
						{
							pPipe->fPendingIO = TRUE;
							continue;
						}

						// An error occurred; disconnect from the client.
						DumpError(pPipe, L"Error while reading 0x%08X", FALSE);
						DisconnectAndReconnect(pPipe);
						continue;
					}
				}

				// WRITING_STATE:
				// The request was successfully read from the client.
				// Get the reply data and write it to the client.
				if (pPipe->dwState == WRITING_STATE)
				{
					GetAnswerToRequest(pPipe);
					fSuccess = WriteFile(
					               pPipe->hPipeInst,
					               pPipe->ptrReply,
					               pPipe->cbReplySize,
					               &cbRet,
					               &pPipe->oOverlap);

					// The write operation completed successfully (w/o pending).
					if (fSuccess /*&& (cbRet == pPipe->cbToWrite)*/)
					{
						_ASSERTE(cbRet == pPipe->cbToWrite);
						DisconnectAndReconnect(pPipe);
						continue;
					}

					// The write operation is still pending.
					dwErr = GetLastError();

					if (!fSuccess && (dwErr == ERROR_IO_PENDING))
					{
						pPipe->fPendingIO = TRUE;
						continue;
					}

					// An error occurred; disconnect from the client.
					DumpError(pPipe, L"Write pipe failed 0x%08X", FALSE);
					pPipe->dwState = ERROR_STATE;
				}

				_ASSERTE(pPipe->dwState<=ERROR_STATE);

				if (mb_Terminate)
				{
					// Server termination requested, exiting
					return 0;
				}

				DisconnectAndReconnect(pPipe);
			}

			// Exiting
			return mb_Terminate ? 0 : -1;
		}
#endif

		int WaitOverlapped(PipeInst* pPipe, DWORD* pcbDataSize)
		{
			BOOL fSuccess = FALSE;
			// Wait for overlapped connection
			HANDLE hConnWait[2] = {mh_TermEvent, pPipe->hEvent};
			DWORD nWait = WaitForMultipleObjects(2, hConnWait, FALSE, INFINITE);
			if (nWait == WAIT_OBJECT_0)
			{
				_ASSERTE(mb_Terminate==TRUE);
				DisconnectNamedPipe(pPipe->hPipeInst);
				return 0; // ���������� �������
			}
			else if (nWait == (WAIT_OBJECT_0+1))
			{
				fSuccess = GetOverlappedResult(pPipe->hPipeInst, &pPipe->oOverlap, pcbDataSize, FALSE);
				_ASSERTE(fSuccess); // ��� ��� ������ ������� ��� �����������/������/������?
				return 1; // OK, ������ �����������
			}
			else
			{
				_ASSERTE(nWait == 0 || nWait == 1);
				DumpError(pPipe, L"PipeServerThread:WaitForMultipleObjects failed with 0x%08X", FALSE);
				// Force client disconnect (error on server side)
				DisconnectNamedPipe(pPipe->hPipeInst);
				// Wait a little
				Sleep(10);
				// Next
				CloseHandle(pPipe->hPipeInst);
				pPipe->hPipeInst = NULL;
				return 2; // ������, ������� ��� ���
			}
		}

		BOOL PipeServerRead(PipeInst* pPipe)
		{
			if (mb_Overlapped)
				ResetEvent(pPipe->hEvent);
			
			int nOverRc;
			DWORD cbRead = 0, dwErr = 0, cbWholeSize = 0;
			BOOL fSuccess = FALSE;
			DWORD In[32];
			T *pIn = NULL;
			SetLastError(0);
			// Send a message to the pipe server and read the response.
			fSuccess = ReadFile(
			               pPipe->hPipeInst, // pipe handle
			               In,               // buffer to receive reply
			               sizeof(In),       // size of read buffer
			               &cbRead,          // bytes read
			               mb_Overlapped ? &pPipe->oOverlap : NULL);
			dwErr = GetLastError();

			if (mb_Overlapped)
			{
				if (!fSuccess || !cbRead)
				{
					nOverRc = WaitOverlapped(pPipe, &cbRead);
					if (nOverRc == 1)
					{
						dwErr = GetLastError();
						fSuccess = TRUE;
					}
					else
					{
						_ASSERTE(mb_Terminate);
						return FALSE; // terminate
					}
				}
			}
			
			if ((!fSuccess && (dwErr != ERROR_MORE_DATA)) || (cbRead < sizeof(DWORD)) || mb_Terminate)
			{
				// ������ �����������?
				//DEBUGSTRPROC(L"!!! ReadFile(pipe) failed - console in close?\n");
				return FALSE;
			}

			//if (in.hdr.nVersion != CESERVER_REQ_VER)
			//{
			//	gConEmu.ShowOldCmdVersion(in.hdr.nCmd, in.hdr.nVersion, in.hdr.nSrcPID==GetServerPID() ? 1 : 0);
			//	return FALSE;
			//}
			//_ASSERTE(in.hdr.cbSize>=sizeof(CESERVER_REQ_HDR) && cbRead>=sizeof(CESERVER_REQ_HDR));
			//if (cbRead < sizeof(CESERVER_REQ_HDR) || /*in.hdr.cbSize < cbRead ||*/ in.hdr.nVersion != CESERVER_REQ_VER) {
			//	//CloseHandle(hPipe);
			//	return;
			//}
			
			cbWholeSize = In[0];

			if (cbWholeSize <= cbRead)
			{
				_ASSERTE(cbWholeSize >= cbRead);
				pIn = pPipe->AllocRequestBuf(cbRead);
				if (!pIn)
				{
					_ASSERTE(pIn!=NULL);
					return FALSE;
				}
				memmove(pIn, In, cbRead);
				pPipe->cbReadSize = cbRead;
			}
			else
			{
				int nAllSize = cbWholeSize;
				pIn = pPipe->AllocRequestBuf(cbWholeSize);
				if (!pIn)
				{
					_ASSERTE(pIn!=NULL);
					return FALSE;
				}
				memmove(pIn, In, cbRead);
				LPBYTE ptrData = ((LPBYTE)pIn)+cbRead;
				nAllSize -= cbRead;

				while (nAllSize>0)
				{
					if (mb_Overlapped)
						ResetEvent(pPipe->hEvent);

					SetLastError(0);
					// Read from the pipe if there is more data in the message.
					fSuccess = ReadFile(
					               pPipe->hPipeInst, // pipe handle
					               ptrData,          // buffer to receive reply
					               nAllSize,         // size of buffer
					               &cbRead,          // number of bytes read
					               mb_Overlapped ? &pPipe->oOverlap : NULL);
					dwErr = GetLastError();

					if (mb_Overlapped)
					{
						if (!fSuccess || !cbRead)
						{
							nOverRc = WaitOverlapped(pPipe, &cbRead);
							if (nOverRc == 1)
							{
								dwErr = GetLastError();
								fSuccess = TRUE;
							}
							else
							{
								_ASSERTE(mb_Terminate);
								return FALSE; // terminate
							}
						}
					}
					
					// Exit if an error other than ERROR_MORE_DATA occurs.
					if (!fSuccess && (dwErr != ERROR_MORE_DATA))
						break;

					ptrData += cbRead;
					nAllSize -= cbRead;
				}

				TODO("Mozhet vozniknutj ASSERT, esli konsolj byla zakryta v processe chtenija");

				if (nAllSize>0)
				{
					_ASSERTE(nAllSize==0);
					return FALSE; // ������� ������� �� ��� ������
				}

				pPipe->cbReadSize = (DWORD)(ptrData - ((LPBYTE)pIn));
			}

			return TRUE;
		}
		
		int PipeServerWrite(PipeInst* pPipe, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten)
		{
			BOOL fWriteSuccess = FALSE, fSuccess;
			
			if (mb_Overlapped)
				ResetEvent(pPipe->hEvent);
			
			fWriteSuccess = WriteFile(pPipe->hPipeInst, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, mb_Overlapped ? &pPipe->oOverlap : NULL);
			
			// The write operation is still pending?
			DWORD dwErr = GetLastError();

			if (mb_Overlapped)
			{
				if (!fWriteSuccess && (dwErr == ERROR_IO_PENDING))
				{
					int nOverRc = WaitOverlapped(pPipe, lpNumberOfBytesWritten);
					if (nOverRc == 0)
						return 0;
					else if (nOverRc == 1)
						return 1;
					else
						return 2;
				}
				else if (fWriteSuccess)
				{
					fSuccess = GetOverlappedResult(pPipe->hPipeInst, &pPipe->oOverlap, lpNumberOfBytesWritten, FALSE);
					_ASSERTE(fSuccess);
					return fSuccess ? 1 : 2;
				}
				else
				{
					return 2;
				}
			}
			else
			{
				return mb_Terminate ? 0 : fWriteSuccess ? 1 : 2;
			}
		}


		// Processing methods
		DWORD PipeServerThread(PipeInst* pPipe)
		{
			_ASSERTE(this && pPipe && pPipe->pServer==this);
			BOOL fConnected = FALSE;
			DWORD dwErr = 0;
			HANDLE hWait[2] = {NULL,NULL}; // ��� Overlapped - ������������ ������ ��� �������� ���������� �������
			DWORD dwTID = GetCurrentThreadId();
			// The main loop creates an instance of the named pipe and
			// then waits for a client to connect to it. When the client
			// connects, a thread is created to handle communications
			// with that client, and the loop is repeated.
			hWait[0] = mh_TermEvent;
			hWait[1] = mh_ServerSemaphore;
			
			_ASSERTE(!mb_Overlapped || pPipe->hEvent != NULL);

			// ���� �� ����������� ���������� �������
			while (!mb_Terminate)
			{
				int   fWriteSuccess = 0;
				BOOL  bNeedReply = FALSE;
				DWORD cbWritten = 0, cbOver = 0;
				
				// ���� �������� �����������
				while (!mb_Terminate)
				{
					_ASSERTE(pPipe->hPipeInst == NULL);
					// !!! ���������� �������� �������� ����� CreateNamedPipe ������, �.�. � ���� ������
					//     ���� ������� � ������ �� ����� ����������� � �������
					// ��������� ���������� ��������, ��� ���������� �������
					dwErr = WaitForMultipleObjects(2, hWait, FALSE, INFINITE);

					if (dwErr == WAIT_OBJECT_0)
					{
						return 0; // ������ �����������
					}
					
					if (mb_Overlapped)
					{
						ResetEvent(pPipe->hEvent);
					}

					_ASSERTE(dwErr == (WAIT_OBJECT_0+1));
					mp_ActivePipe = pPipe;
					DWORD dwOpenMode = PIPE_ACCESS_DUPLEX |                      // read/write access
						(mb_Overlapped ? FILE_FLAG_OVERLAPPED : 0); // overlapped mode
					DWORD dwPipeMode = PIPE_TYPE_MESSAGE |         // message type pipe
						PIPE_READMODE_MESSAGE |     // message-read mode
						PIPE_WAIT;
					int Count = MaxCount;
					DWORD nOutSize = PIPEBUFSIZE, nInSize = PIPEBUFSIZE;
					_ASSERTE(mlp_Sec!=NULL);
					pPipe->hPipeInst = CreateNamedPipeW(ms_PipeName, dwOpenMode, dwPipeMode, Count, nOutSize, nInSize, 0, mlp_Sec);

					if (pPipe->hPipeInst == INVALID_HANDLE_VALUE)
					{
						dwErr = GetLastError();
						
						_ASSERTE(pPipe->hPipeInst != INVALID_HANDLE_VALUE);
						DumpError(pPipe, L"PipeServerThread:ConnectNamedPipe failed with 0x%08X", FALSE);
						
						//DisplayLastError(L"CreateNamedPipe failed");
						pPipe->hPipeInst = NULL;
						// Wait a little
						Sleep(10);
						// ��������� ���� ��� ������ ���� ������� ��������� ����
						ReleaseSemaphore(hWait[1], 1, NULL);
						// Try again
						continue;
					}

					// ����� ConEmuC ����, ��� ��������� ���� �����
					if (!mb_ReadyCalled && mfn_PipeServerReady)
					{
						mb_ReadyCalled = TRUE;
						mfn_PipeServerReady(m_lParam);
						//SetEvent(pRCon->mh_GuiAttached);
						//SafeCloseHandle(pRCon->mh_GuiAttached);
					}

					// Wait for the client to connect; if it succeeds,
					// the function returns a nonzero value. If the function
					// returns zero, GetLastError returns ERROR_PIPE_CONNECTED.
					SetLastError(0);
					fConnected = ConnectNamedPipe(pPipe->hPipeInst, mb_Overlapped ? &pPipe->oOverlap : NULL);
					dwErr = GetLastError();
					             //? TRUE : ((dwErr = GetLastError()) == ERROR_PIPE_CONNECTED);
					// ����� ��������� ������ ���� ������� �����
					ReleaseSemaphore(hWait[1], 1, NULL);
					
					bool lbFail = false;
					if (mb_Overlapped)
					{
						// Overlapped ConnectNamedPipe should return zero.
						if (fConnected)
						{
							DumpError(pPipe, L"PipeServerThread:ConnectNamedPipe failed with 0x%08X", FALSE);
							Sleep(100);
							continue;
						}
						bNeedReply = (dwErr == ERROR_PIPE_CONNECTED);
					}
					else
					{
						if (!fConnected && dwErr == ERROR_PIPE_CONNECTED)
							fConnected = TRUE;
						bNeedReply = fConnected;
					}

					// ������ �����������?
					if (mb_Terminate)
						break;
					
					// ��������� �����������
					if (mb_Overlapped)
					{
						// Wait for overlapped connection
						int nOverRc = WaitOverlapped(pPipe, &cbOver);
						if (nOverRc == 0)
						{
							break; // ���������� �������
						}
						else if (nOverRc == 1)
						{
							bNeedReply = TRUE; // OK, ������ �����������
							break;
						}
						else
						{
							continue; // ������, ������� ��� ���
						}
					} // end - mb_Overlapped
					else
					{
						if (fConnected)
						{
							break; // OK, ����� �� ����� �������� �����������, ������� � ������
						}
						else
						{
							DisconnectNamedPipe(pPipe->hPipeInst);
							CloseHandle(pPipe->hPipeInst);
							pPipe->hPipeInst = NULL;
						}
					} // end - !mb_Overlapped
				}
				
				// ������ �����������?
				if (mb_Terminate)
				{
					// ������ ����-������ �������� � ����, � �� ������ (Pipe was closed) � ������� ���������
					if (pPipe->hPipeInst)
					{
						if (bNeedReply)
						{
							DWORD dwNil = 0, cbSize = sizeof(DWORD);
							fWriteSuccess = PipeServerWrite(pPipe, &dwNil, cbSize, &cbWritten);
							
							// Function does not return until the client process has read all the data.
							//FlushFileBuffers(pPipe->hPipeInst);
						}
						//DisconnectNamedPipe(hPipe);
						CloseHandle(pPipe->hPipeInst);
						pPipe->hPipeInst = NULL;
					}
					return 0;
				}
				
				TODO("Po nastrojke, zaciklit'sja na chtenii, ne zakryvatj podkljuchenie posle odnoj komandy");
				
				BOOL lbFirstRead = TRUE;
				BOOL lbAllowClient = TRUE;
				
				if (mfn_PipeServerConnected)
				{
					if (!mfn_PipeServerConnected(m_lParam, pPipe->hPipeInst))
					{
						lbAllowClient = FALSE; // ������ ����������� �� ����� �������
					}
				}
				

				if (lbAllowClient)
				{
					// ������ ������ ������� �� �����
					while (PipeServerRead(pPipe))
					{
						if (lbFirstRead)
						{
							lbFirstRead = FALSE;
						}
						
						if (mfn_PipeServerCommand(pPipe->ptrRequest, /*OUT&*/pPipe->ptrReply,
									/*OUT&*/pPipe->cbReplySize, /*OUT&*/pPipe->cbMaxReplySize, m_lParam, pPipe->hPipeInst))
						{
							// ���� ������� - ������ � ���� ����������
							fWriteSuccess = PipeServerWrite(pPipe, pPipe->ptrReply, pPipe->cbReplySize, &cbWritten);
						}
						else
						{
							// ������ ����-������ �������� � ����, � �� ������ (Pipe was closed) � ������� ���������
							DWORD dwNil = 0, cbSize = sizeof(DWORD);
							fWriteSuccess = PipeServerWrite(pPipe, &dwNil, cbSize, &cbWritten);
						}
					
						if (mb_Terminate || !mb_LoopCommands)
							break;
					}
				}
				
				// Fin
				if (lbFirstRead)
				{
					// ������ ����-������ �������� � ����, � �� ������ (Pipe was closed) � ������� ���������
					DWORD dwNil = 0, cbSize = sizeof(DWORD);
					fWriteSuccess = PipeServerWrite(pPipe, &dwNil, cbSize, &cbWritten);
				}
				
				if (!mb_Terminate)
				{
					// Function does not return until the client process has read all the data.
					FlushFileBuffers(pPipe->hPipeInst);
				}
				
				if (mfn_PipeServerDisconnected)
					mfn_PipeServerDisconnected(m_lParam, pPipe->hPipeInst);
				
				TODO("DisconnectAndReconnect?");
				//DisconnectNamedPipe(pPipe->hPipeInst);
				CloseHandle(pPipe->hPipeInst);
				pPipe->hPipeInst = NULL;
				// ������� � �������� ������ instance �����
			}

			return 0;
		};
		static DWORD WINAPI _PipeServerThread(LPVOID lpvParam)
		{
			DWORD nResult = 0;
			PipeInst* pPipe = (PipeInst*)lpvParam;
			_ASSERTE(pPipe!=NULL && pPipe->pServer!=NULL);
			nResult = pPipe->pServer->PipeServerThread(pPipe);
			return nResult;
		};
	//public:
	//	CPipeServer()
	//	{
	//		mb_Initialized = FALSE;
	//		ms_PipeName[0] = 0;
	//		memset(m_Pipes, 0, sizeof(m_Pipes));
	//		mp_ActivePipe = NULL;
	//		mfn_PipeServerCommand = NULL;
	//		mfn_PipeServerReady = NULL;
	//		mb_ReadyCalled = FALSE;
	//		m_lParam = 0;
	//		mh_TermEvent = NULL;
	//		mb_Terminate = FALSE;
	//		mh_ServerSemaphore = NULL;
	//	};
	//	~CPipeServer()
	//	{
	//		StopPipeServer();
	//	};
	public:
		BOOL StartPipeServer(
			    LPCWSTR asPipeName,
			    LPARAM alParam,
			    LPSECURITY_ATTRIBUTES alpSec /*= LocalSecurity()*/, // MUST! be alive for life cicle
			    PipeServerCommand_t apfnPipeServerCommand,
			    PipeServerFree_t apfnPipeServerFree = NULL,
			    PipeServerConnected_t apfnPipeServerConnected = NULL,
			    PipeServerConnected_t apfnPipeServerDisconnected = NULL,
			    PipeServerReady_t apfnPipeServerReady = NULL,
			    BOOL abOverlapped = FALSE, // ����
			    BOOL abLoopCommands = FALSE
		    )
		{
			if (mb_Initialized)
			{
				_ASSERTE(mb_Initialized==FALSE);
				return FALSE;
			}
			
			mb_Initialized = FALSE;
			mb_Overlapped = abOverlapped;
			mb_LoopCommands = abLoopCommands;
			if (MaxCount < 1)
				MaxCount = 1;
			_ASSERTE(MaxCount<=countof(m_Pipes));
			memset(m_Pipes, 0, sizeof(m_Pipes));
			mp_ActivePipe = NULL;
			mb_ReadyCalled = FALSE;
			mh_TermEvent = CreateEvent(NULL,TRUE/*MANUAL - ������������ � ���������� �����!*/,FALSE,NULL); ResetEvent(mh_TermEvent);
			mb_Terminate = FALSE;
			mh_ServerSemaphore = CreateSemaphore(NULL, 1, 1, NULL);
			
			wcscpy_c(ms_PipeName, asPipeName);
			m_lParam = alParam;
			mlp_Sec = alpSec;
			
			mfn_PipeServerCommand = apfnPipeServerCommand;
			mfn_PipeServerReady = apfnPipeServerReady;
			mfn_PipeServerConnected = apfnPipeServerConnected;
			mfn_PipeServerDisconnected = apfnPipeServerDisconnected;
			mfn_PipeServerFree = apfnPipeServerFree;
			_ASSERTE(mb_ReadyCalled==FALSE);
			
			// ���������� ������� ��� ������� ��� �������
			if (!apfnPipeServerCommand)
			{
				//_ASSERTE(apfnPipeServerCommand!=NULL);
				DumpError(&(m_Pipes[0]), L"StartPipeServer:apfnPipeServerCommand is NULL");
				return FALSE;
			}
			
			if (mb_Overlapped)
			{
				for (int j = 0; j < MaxCount; j++)
				{
					m_Pipes[j].hEvent = CreateEvent(
					                        NULL,    // default security attribute
					                        TRUE,    // manual-reset(!) event
					                        TRUE,    // initial state = signaled
					                        NULL);   // unnamed event object

					if (m_Pipes[j].hEvent == NULL)
					{
						DumpError(&(m_Pipes[j]), L"StartPipeServer:CreateEvent failed with 0x%08X");
						return -(200+j);
					}

					m_Pipes[j].oOverlap.hEvent = m_Pipes[j].hEvent;
				}
			}
			
			for (int i = 0; i < MaxCount; i++)
			{
				_ASSERTE(m_Pipes[i].hThread == NULL);

				m_Pipes[i].pServer = this;
				m_Pipes[i].nThreadId = 0;
				m_Pipes[i].hThread = CreateThread(NULL, 0, _PipeServerThread, (LPVOID)&(m_Pipes[i]), 0, &m_Pipes[i].nThreadId);
				if (m_Pipes[i].hThread == NULL)
				{
					//_ASSERTE(m_Pipes[i].hThread!=NULL);
					DumpError(&(m_Pipes[i]), L"StartPipeServer:CreateThread failed, code=0x%08X");
					return FALSE; // �� ������� ������� ��������� ������
				}
			}
			
			#if 0
			// Overlapped, ��������
			// ������ Instances
			int iRc = InitPipes();
			return (iRc == 0);
			#endif
			
			return TRUE;
		};
		void StopPipeServer()
		{
			DWORD nWait;
			
			mb_Terminate = TRUE;
			if (mh_TermEvent)
				SetEvent(mh_TermEvent);

			for (int i = 0; i < MaxCount; i++)
			{
				if (m_Pipes[i].hThread)
				{
					nWait = WaitForSingleObject(m_Pipes[i].hThread, 250);

					if (nWait != WAIT_OBJECT_0)
					{
						_ASSERTE(nWait == WAIT_OBJECT_0);
						TerminateThread(m_Pipes[i].hThread, 100);
					}

					CloseHandle(m_Pipes[i].hThread);
					m_Pipes[i].hThread = NULL;
				}

				if (m_Pipes[i].hPipeInst && (m_Pipes[i].hPipeInst != INVALID_HANDLE_VALUE))
				{
					DisconnectNamedPipe(m_Pipes[i].hPipeInst);
					CloseHandle(m_Pipes[i].hPipeInst);
				}
				m_Pipes[i].hPipeInst = NULL;
				
				if (m_Pipes[i].hEvent)
				{
					CloseHandle(m_Pipes[i].hEvent);
					m_Pipes[i].hEvent = NULL;
				}
				
				if (m_Pipes[i].ptrRequest)
				{
					free(m_Pipes[i].ptrRequest);
					m_Pipes[i].ptrRequest = NULL;
				}
				
				if (m_Pipes[i].ptrReply && mfn_PipeServerFree)
				{
					mfn_PipeServerFree(m_Pipes[i].ptrReply, m_lParam);
					m_Pipes[i].ptrReply = NULL;
				}
			}
			
			if (mh_ServerSemaphore)
			{
				CloseHandle(mh_ServerSemaphore);
				mh_ServerSemaphore = NULL;
			}
			if (mh_TermEvent)
			{
				CloseHandle(mh_TermEvent);
				mh_TermEvent = NULL;
			}
		};
};
