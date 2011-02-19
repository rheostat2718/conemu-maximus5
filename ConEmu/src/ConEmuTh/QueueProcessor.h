
/*
Copyright (c) 2010 Maximus5
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

#ifndef TRY
#define TRY __try
#define CATCH __except(EXCEPTION_EXECUTE_HANDLER)
#endif

//#ifndef sizeofarray
//	#define sizeofarray(x) (sizeof(x)/sizeof(*x))
//#endif

enum ProcessingPriority
{
	ePriorityHighest = 0, // ���� ������� ����� ������������ �����
	ePriorityAboveNormal,
	ePriorityNormal,
	ePriorityBelowNormal,
	ePriorityLowest,
};

enum ProcessingStatus
{
	eItemEmpty = 0,
	eItemPassed, // ����� ��-�� ��� � eItemEmpty, �� ���������
	eItemFailed,
	eItemRequest,
	eItemReady,
	eItemProcessing, // NOW!
};

template<class T>
class CQueueProcessor
{
	private:
		// typedefs
		struct ProcessingItem
		{
			int      Status;  // enum ProcessingStatus
			int      Priority;// enum ProcessingPriority
			HRESULT  Result;  // ��������� �� ProcessItem
			bool     Synch;   // ��������������� � true ��� ���������� ���������
			//HANDLE Ready;   // Event, ����� �������������� ��� �������� ����������
			DWORD    Start;   // Tick ������ ���������
			// ���� ��������� ������� ��������� ������ ��������� �������� -
			// �������� ������, ������-������������, ������� ������� ���������
			// ��� ������� ����� �������� �������, ��� ��������� ������� ��
			// ������-������������ �������. �������� ��������� �������� ��
			// ������� �� ���������� - �� ����� ����� ��������� ��������.
			bool     StopRequested;
			// ���������� �� ��������
			T        Item;    // ����� ������ �� RequestItem
			LONG_PTR lParam;  // ��� ���������� ���� ��������
		};
		// ������� ���������
		CRITICAL_SECTION mcs_Queue;
		int mn_Count, mn_MaxCount;
		int mn_WaitingCount;
		HANDLE mh_Waiting; // Event ��� �������������� ���� �����������
		ProcessingItem** mpp_Queue; // **, ����� �� ������������ �� �������� ��������
		HANDLE mh_Queue; DWORD mn_QueueId;
		HANDLE mh_Alive; DWORD mn_AliveTick;
		// �������� ������� (�������������� ������)
		ProcessingItem* mp_Active;  // !!! � NULL �� ���������� !!!
		ProcessingItem* mp_SynchRequest; // ���� ������ ���� "����������" ������

		// ������ �� Terminate ���� ���������
		bool mb_TerminateRequested;

	private:
		// �������� ������ �� �������� �������. ���� �������� ��� �������� -
		// ������� ������������ �� ������������. ����������� ����� (������)
		// ������, ���� anInitialCount ��������� mn_MaxCount
		// ������� ��������� ����� ������� �� ���������.
		// ������ ��� ������ ���� �������������
		void InitializeQueue(int anInitialCount)
		{
			if (anInitialCount < mn_MaxCount && mpp_Queue)
			{
				// ��� �������� ���������� ���������
				return;
			}

			ProcessingItem** pp = (ProcessingItem**)calloc(anInitialCount,sizeof(ProcessingItem*));

			if (mpp_Queue && mn_Count)
			{
				_ASSERTE(mn_Count <= mn_MaxCount);
				memmove(pp, mpp_Queue, mn_Count*sizeof(ProcessingItem*));
			}

			if (mpp_Queue)
				free(mpp_Queue);
			mpp_Queue = pp;

			for(int i = mn_MaxCount; i < anInitialCount; i++)
			{
				mpp_Queue[i] = (ProcessingItem*)calloc(1,sizeof(ProcessingItem));
			}

			mn_MaxCount = anInitialCount;
			// Done
		};
	private:
		// ������ ������� �������. �� ����������� �� ����� ����� ����!
		void ClearQueue()
		{
			_ASSERTE(mh_Queue==NULL || WaitForSingleObject(mh_Queue,0)==WAIT_OBJECT_0);

			if (mpp_Queue)
			{
				_ASSERTE(eItemEmpty == 0);

				for(int i = 0; i < mn_MaxCount; i++)
				{
					if (mpp_Queue[i])
					{
						if (mpp_Queue[i]->Status != eItemEmpty && mpp_Queue[i]->Status != eItemPassed)
						{
							FreeItem(mpp_Queue[i]->Item);
						}

						free(mpp_Queue[i]);
						mpp_Queue[i] = NULL;
					}
				}

				free(mpp_Queue);
				mpp_Queue = NULL;
			}
		};
	private:
		// Warning!!! ������ ���������� ������ CriticalSection
		void CheckWaitingCount()
		{
			int nWaiting = 0;

			for(int i = 0; i < mn_Count; i++)
			{
				if (mpp_Queue[i]->Status == eItemRequest)
					nWaiting ++;
			}

			mn_WaitingCount = nWaiting;

			if (mn_WaitingCount > 0)
				SetEvent(mh_Waiting);
			else
				ResetEvent(mh_Waiting);
		}
		// ����� � ������� ������� (���� ��� ��� ���������) ��� �������� � ������� �����
		ProcessingItem* FindOrCreate(const T& pItem, LONG_PTR lParam, bool Synch, ProcessingPriority Priority)
		{
			if (GetCurrentThreadId() == mn_QueueId)
			{
				_ASSERTE(GetCurrentThreadId() != mn_QueueId);
			}

			int i;
			ProcessingItem* p = NULL;
			EnterCriticalSection(&mcs_Queue);

			if (!mpp_Queue || !mn_MaxCount)
			{
				// ������� ��� �� ���� ����������������
				InitializeQueue(64);
			}
			else
			{
				// ���� ����� ������ ��� ������ - ������ �������� ��� ���������/�����?
				for(i = 0; i < mn_Count; i++)
				{
					//if (mpp_Queue[i]->Status != eItemEmpty
					//	&& mpp_Queue[i]->Status != eItemPassed
					//	&& mpp_Queue[i]->Status != eItemFailed)
					if (mpp_Queue[i]->Status == eItemRequest)
					{
						if (IsEqual(pItem, lParam, (mpp_Queue[i]->Item), mpp_Queue[i]->lParam))
						{
							// ����� ���� ���������� ���������
							mpp_Queue[i]->Priority = Priority;
							mpp_Queue[i]->lParam = lParam;
							mpp_Queue[i]->Synch = Synch;
							ApplyChanges(mpp_Queue[i]->Item, pItem);
							//SetEvent(mh_Waiting);
							CheckWaitingCount();
							_ASSERTE(mpp_Queue[i]->Status != eItemEmpty);
							LeaveCriticalSection(&mcs_Queue);
							//if (mpp_Queue[i]->Status == eItemRequest) -- ������������ ����� ���� ��� ��������, �� ����� - �� �������
							return mpp_Queue[i];
						}
					}
					else if (!p && (mpp_Queue[i]->Status == eItemEmpty || mpp_Queue[i]->Status == eItemPassed))
					{
						p = mpp_Queue[i]; // ������ �������, ���� �� ����� "��� ��"
					}
				}

				// ���� "������" ������ �� �����, � ��������� �� ��������
				if (p == NULL && mn_Count >= mn_MaxCount)
				{
					_ASSERTE(mn_Count == mn_MaxCount);
					InitializeQueue(mn_MaxCount+64);
				}
			}

			// ����� ������?
			if (!p)
			{
				if (mn_Count < mn_MaxCount)
				{
					p = mpp_Queue[mn_Count++];
				}

				_ASSERTE(p != NULL);
			}

			// �������� ������
			if (p)
			{
				p->Priority = Priority;
				p->lParam = lParam;
				p->Result = S_OK;
				p->Start = 0;
				p->StopRequested = false;
				p->Synch = Synch;
				CopyItem(&pItem, &p->Item);
				// ���������
				_ASSERTE(p->Status == eItemEmpty || p->Status == eItemPassed);
				p->Status = eItemRequest;
				mn_WaitingCount ++;
				SetEvent(mh_Waiting);
				_ASSERTE(p->Status != eItemEmpty);
			}

			CheckWaitingCount();
			LeaveCriticalSection(&mcs_Queue);
			return p;
		};
	public:
		CQueueProcessor()
		{
			InitializeCriticalSection(&mcs_Queue);
			mn_Count = mn_MaxCount = mn_WaitingCount = 0;
			mh_Waiting = NULL;
			mpp_Queue = NULL;
			mh_Queue = NULL; mn_QueueId = 0;
			mp_Active = mp_SynchRequest = NULL;
			mh_Alive = NULL; mn_AliveTick = NULL;
		};
		~CQueueProcessor()
		{
			Terminate(250);

			if (mh_Queue)
			{
				CloseHandle(mh_Queue); mh_Queue = NULL;
			}

			if (mh_Alive)
			{
				CloseHandle(mh_Alive);
				mh_Alive = NULL;
			}

			DeleteCriticalSection(&mcs_Queue);
			ClearQueue();

			if (mh_Waiting)
			{
				CloseHandle(mh_Waiting);
				mh_Waiting = NULL;
			}
		};

	public:
		// ������ �� ���������� �������
		void RequestTerminate()
		{
			mb_TerminateRequested = true;
			SetEvent(mh_Waiting);
		};

	public:
		// ����������� ���������� �������. ������� ������ nTimeout �� ���������� �������.
		void Terminate(DWORD nTimeout = INFINITE)
		{
			if (!mh_Queue)
			{
				// ���
				return;
			}

			// ��������� ����
			RequestTerminate();

			// �����������
			if (GetCurrentThreadId() == mn_QueueId)
			{
				_ASSERTE(GetCurrentThreadId() != mn_QueueId);
				return;
			}

			// ��������� ����������
			DWORD nWait = WaitForSingleObject(mh_Queue, nTimeout);

			if (nWait != WAIT_OBJECT_0)
			{
				_ASSERTE(nWait == WAIT_OBJECT_0);
				TerminateThread(mh_Queue, 100);
			}
		};
	public:
		// ���� ����������� ���������� "�������" ���� ���������
		bool IsAlive()
		{
			if (!mh_Alive)
				return false;

			ResetEvent(mh_Alive);
			DWORD nDelta = (GetTickCount() - mn_AliveTick);

			if (nDelta < 100)
				return true;

			//TODO: �������� �������� �������� ������, ���������
			if (WaitForSingleObject(mh_Alive, 500) == WAIT_OBJECT_0)
				return true;

			return false;
		}
	public:
		// ��������!!! ���������� pItem ���������� �� ���������� �����.
		// ��� ������ ProcessItem �������� ��������� �� ����� ������� �� ����������� ������.
		// ���� ����� �������� �������������� ��������� - ����������� lParam
		// ���� Synch==true - ������� ������� ���������� ��������� ��������, � Priority ��������������� � ePriorityHighest
		// ������� - ��� (Synch==true) ��������� ProcessItem(...)
		//           ��� (Synch==false) - S_FALSE, ���� ��� � �������, ����� - ��������� ProcessItem(...)
		// !!! ��� ���������� ������� - ��������� ������������ ����� pItem.
		HRESULT RequestItem(bool Synch, ProcessingPriority Priority, const T pItem, LONG_PTR lParam)
		{
			HRESULT hr = CheckThread();

			if (FAILED(hr))
				return hr;

			ProcessingItem* p = FindOrCreate(pItem, lParam, Synch, Synch ? ePriorityHighest : Priority);

			if (!p)
			{
				_ASSERTE(p!=NULL);
				return E_UNEXPECTED;
			}

			_ASSERTE(p->Status != eItemEmpty);

			// ���� ����� ��������� "������"
			if (Synch)
			{
				// ���� ��� ��������� - ����� ������ ���������.
				if (p->Status == eItemPassed)
				{
					// ��� ��������� ����������
					return p->Result;
				}

				// ���������� ��������� �� "����������" ������
				mp_SynchRequest = p;

				// ��������, ���� �� �������� �������?
				if (mp_Active && mp_Active != p)
				{
					// ������ � ���� ��������� �������������� ������ �������
					// ���������� ���������� ��� �������� (���� ��������) ��� ���������
					mp_Active->StopRequested = true;
				}

				//// ��� �������� - ��������� Event
				//if (p->Ready)
				//	ResetEvent(p->Ready);
				//else
				//	p->Ready = CreateEvent(NULL, FALSE, FALSE, NULL);
				// ������� ����������
				DWORD nWait = -1;

				//HANDLE hEvents[2] = {p->Ready, mh_Queue};
				while((nWait = WaitForSingleObject(mh_Queue,10)) == WAIT_TIMEOUT)
				{
					//nWait = WaitForMultipleObjects(2, hEvents, FALSE, 25);
					//if (nWait == (WAIT_OBJECT_0 + 1))
					//{
					//	// ���� ��������� ���� ���������
					//	return E_ABORT;
					//}
					//if (p->Status == eItemPassed)
					//{
					//	// ��� ��������� ����������
					//	return p->Result;
					//}
					if (p->Status == eItemPassed ||
					        (p->Status == eItemFailed || p->Status == eItemReady))
					{
						if (mp_SynchRequest == p)
						{
							mp_SynchRequest = NULL;
						}

						//CloseHandle(p->Ready);
						//p->Ready = NULL;
						//// ������� ��������� ��������� (������)
						//CopyItem(&p->Item, &pItem);
						//// � ����� ����� ���������� ������
						//memset(&p->Item, 0, sizeof(p->Item));
						//p->Status = eItemEmpty;
						// ������� ��������
						return p->Result;
					}

					//if (nWait != WAIT_TIMEOUT)
					//{
					//	_ASSERTE(nWait == WAIT_TIMEOUT);
					//	return E_ABORT;
					//}
				}

				// ���� ��������� ���� ���������
				return E_ABORT;
			}

			// ��� ���������� - ��������� ������������ ����� OnItemReady / OnItemFailed
			return S_FALSE;
			//if (p->Status == eItemRequest || p->Status == eItemProcessing)
			//	return S_FALSE; // � ��������
			//
			//if (p->Status != eItemFailed && p->Status != eItemReady)
			//{
			//	_ASSERTE(p->Status == eItemFailed || p->Status == eItemReady);
			//	return E_UNEXPECTED;
			//}
			//// ������� ��������� ��������� (������)
			//CopyItem(&p->Item, pItem);
			//// � ����� ����� ���������� ������
			//memset(&p->Item, 0, sizeof(p->Item));
			//p->Status = eItemEmpty;
			//// ������� ��������
			//return p->Result;
		};

		// ����� ���������� ����� ���������� ��������� � ������� �����
		// ��������� ��������� ������� ��������� �� ���������� ��������
		// ������������ ������� ResetQueue
		void DiscountPriority(UINT nSteps = 1)
		{
			if (mpp_Queue)
			{
				_ASSERTE(eItemEmpty == 0);
				// ��� ��������� �������� - ����� ������������� ������
				EnterCriticalSection(&mcs_Queue);

				for(int i = 0; i < mn_MaxCount; i++)
				{
					if (mpp_Queue[i])
					{
						if (mpp_Queue[i]->Status != eItemEmpty
						        && mpp_Queue[i]->Status != eItemPassed
						        && mpp_Queue[i]->Status != eItemFailed)
						{
							int nNew = min(ePriorityLowest, (mpp_Queue[i]->Priority+nSteps));
							mpp_Queue[i]->Priority = nNew;
						}
					}
				}

				CheckWaitingCount();
				// ������ �� ���������
				LeaveCriticalSection(&mcs_Queue);
			}
		};

		// ��������� ��� ������, � ����������� ������ ��� ���� ����������
		// ������������ ������� DiscountPriority
		void ResetQueue(ProcessingPriority priority = ePriorityHighest)
		{
			// -- ������� - ������� -- �� �����
			//Terminate();

			// ����� �����, ��������������� �������
			if (mpp_Queue)
			{
				_ASSERTE(eItemEmpty == 0);
				// ��� ��������� ���������� �������� - ����� ������������� ������
				EnterCriticalSection(&mcs_Queue);

				for(int i = 0; i < mn_MaxCount; i++)
				{
					if (mpp_Queue[i])
					{
						if (mpp_Queue[i]->Status != eItemEmpty
						        mpp_Queue[i]->Status != eItemPassed
						        && mpp_Queue[i]->Status != eItemProcessing
						        && mpp_Queue[i]->Priority >= priority
						  )
						{
							FreeItem(&mpp_Queue[i]->Item);
						}

						memset(mpp_Queue[i], 0, sizeof(ProcessingItem));
					}
				}

				CheckWaitingCount();
				// ������ �� ���������
				LeaveCriticalSection(&mcs_Queue);
			}

			//mn_Count = 0; -- !!! ������ !!!
		};

		// ���� ��������� ������� ��������� ������ ��������� �������� -
		// �������� ������, ������-������������, ������� ������� ���������
		// ��� ������� ����� �������� �������, ��� ��������� ������� ��
		// ������-������������ �������. �������� ��������� �������� ��
		// ������� �� ���������� - �� ����� ����� ��������� ��������.
		virtual bool IsStopRequested(const T& pItem)
		{
			_ASSERTE(mp_Active && pItem == mp_Active->Item);

			if (!mp_Active)
				return false;

			return mp_Active->StopRequested;
		};

	protected:
		/* *** ��������! ������������� ��������������� � �������� *** */

		// ��������� ��������. ������� ������ ����������:
		// S_OK    - ������� ������� ���������, ����� ���������� ������ eItemReady
		// S_FALSE - ������ ���������, ����� ���������� ������ eItemFailed
		// FAILED()- ������ eItemFailed � ���� ����������� ����� ���������
		virtual HRESULT ProcessItem(T& pItem, LONG_PTR lParam)
		{
			return E_NOINTERFACE;
		};

	protected:
		/* *** ����� �������������� � �������� *** */

		// ���������� ��� �������� ���������� ��������� �������� ��� ����������� ���������.
		// ���� ������� ��������� ������� (Status == eItemReady), ���������� OnItemReady
		virtual void OnItemReady(T& pItem, LONG_PTR lParam)
		{
			return;
		};
		// ����� (Status == eItemFailed) - OnItemFailed
		virtual void OnItemFailed(T& pItem, LONG_PTR lParam)
		{
			return;
		};
		// ����� ���������� ���� ������� ������ ���������!

	protected:
		/* *** ����� �������������� � �������� *** */

		// ���� ��������� ������� ���� �������� � ����� �� ���� ����������
		virtual bool IsTerminationRequested()
		{
			return mb_TerminateRequested;
		};
		// ����� ������� ����� ��������� CoInitialize ��������
		virtual HRESULT OnThreadStarted()
		{
			return S_OK;
		}
		// ����� ������� ����� ��������� CoUninitialize ��������
		virtual void OnThreadStopped()
		{
			return;
		};
		// ���� ��������� ������������� �������� �� ����������� �������� - ��������������
		virtual void CopyItem(const T* pSrc, T* pDst)
		{
			if (pSrc != pDst)
				memmove(pDst, pSrc, sizeof(T));
		};
		// ���� ������� ��� ��� �������� � ������� �����������, � ������ ������ ����� ������
		virtual void ApplyChanges(T& pDst, const T& pSrc)
		{
			return;
		}
		// ����� �������������� ��� ��������� ������ ��������� (������������ ��� ������)
		virtual bool IsEqual(const T& pItem1, LONG_PTR lParam1, const T& pItem2, LONG_PTR lParam2)
		{
			int i = memcmp(pItem1, pItem2, sizeof(T));
			return (i == 0) && (lParam1 == lParam2);
		};
		// ���� � T ���������� �����-�� ��������� - ���������� ��
		virtual void FreeItem(const T& pItem)
		{
			return;
		};
		// ���� ������� ������� ������������ - ���� �� ������������������
		virtual bool CheckHighPriority(const T& pItem)
		{
			// ��������� � ������� � ������� false, ����, ��������, ��� ������
			// ��� ������� ��������, �� ������������ ��� ������� � ��� �� ������
			return true;
		};

	protected:
		/* *** ��� ������� �� �������������� *** */
		bool ProcessingStep()
		{
			// �� ����
			mn_AliveTick = GetTickCount();
			SetEvent(mh_Alive);
			// ���� ����� ������������
			ProcessingItem* p = NULL;

			if (WaitForSingleObject(mh_Waiting, 50) == WAIT_TIMEOUT)
			{
				//_ASSERTE(mn_WaitingCount==0);
				if (mn_WaitingCount == 0)
					return false; // true==Terminate
			}

			// ��� ��������� ���������� �������� - ����� ������������� ������
			EnterCriticalSection(&mcs_Queue);

			// ���������?
			if (mp_SynchRequest == NULL)
			{
				// ����� ����������� ������
				for(int piority = ePriorityHighest; piority <= ePriorityLowest; piority++)
				{
					for(int i = 0; i < mn_Count; i++)
					{
						if (piority <= ePriorityAboveNormal && mpp_Queue[i]->Priority <= piority)
						{
							if (!CheckHighPriority(mpp_Queue[i]->Item))
							{
								// ������� �������� ���� ������������������
								mpp_Queue[i]->Priority = ePriorityNormal;
								continue;
							}
						}

						if (mpp_Queue[i]->Status == eItemRequest  // ����� "�������������"
						        // � ��������������� �����������
						        && (mpp_Queue[i]->Priority == piority || piority == ePriorityLowest))
						{
							p = mpp_Queue[i]; break;
						}
					}

					// ���� ����� - ����� ������
					if (p) break;
				}

				// ����� ���� ������� ��� ��������� ����������?
				if (p && p->StopRequested)
				{
					p->StopRequested = false;
					p = NULL;
				}
			}

			// ����� ���� ������ ������ "����������" ������?
			if (mp_SynchRequest)
			{
				p = mp_SynchRequest;
				mp_SynchRequest = NULL; // ����� ��������, ������ �� �����
			}

			if (p)  // ����� ��������� ������, �� ������ �� ������
			{
				p->Status = eItemProcessing;
			}

			CheckWaitingCount();
			// ������ ������ �� �����
			LeaveCriticalSection(&mcs_Queue);

			// ����� ��� ��� ������ �� Terminate?
			if (IsTerminationRequested())
				return true;

			// ���� ����, ���� ����������
			if (p)
			{
				HRESULT hr = E_FAIL;
				// ����� �����, ��� ��������������
				mp_Active = p;
				// ��������� �����, ��� ���� ������� "�����" � �����
				p->Start = GetTickCount();
				p->Status = eItemProcessing;
				TRY
				{
					// ����������, ���������. ����������� � �������
					hr = ProcessItem(p->Item, p->lParam);
				}
				CATCH
				{
					hr = E_UNEXPECTED;
				}
				_ASSERTE(hr!=E_NOINTERFACE);
				p->Result = hr;
				p->Status = (hr == S_OK) ? eItemReady : eItemFailed;

				// ������������ � "����������", ���� ������ ��� �����������
				if (!p->Synch)
				{
					// ������� ��������� ��������� (������)
					if (hr == S_OK)
						OnItemReady(p->Item, p->lParam);
					else
						OnItemFailed(p->Item, p->lParam);

					// � ����� ����� ���������� ������
					memset(p->Item, 0, sizeof(p->Item));
					p->Status = eItemPassed;
				}

				//if (FAILED(hr)) -- �� ����� ��� �������. ���� ����� - ������� RequestTerminate.
				//{
				//	_ASSERTE(SUCCEEDED(hr));
				//	RequestTerminate();
				//	return true;
				//}
			}

			return IsTerminationRequested();
		};
		static DWORD CALLBACK ProcessingThread(LPVOID pParam)
		{
			CQueueProcessor<T>* pThis = (CQueueProcessor<T>*)pParam;

			while(!pThis->IsTerminationRequested())
			{
				if (pThis->ProcessingStep())
					break;
			}

			return 0;
		};
		HRESULT CheckThread()
		{
			// ����� ���� ��� ���������?
			if (mh_Queue && mn_QueueId)
			{
				if (WaitForSingleObject(mh_Queue, 0) == WAIT_OBJECT_0)
				{
					CloseHandle(mh_Queue);
					mh_Queue = NULL;
				}
			}

			if (!mh_Alive)
			{
				mh_Alive = CreateEvent(NULL, FALSE, FALSE, NULL);
				mn_AliveTick = 0;
			}

			if (!mh_Waiting)
			{
				mh_Waiting = CreateEvent(NULL, TRUE, FALSE, NULL);
			}

			if (!mh_Queue)
			{
				mb_TerminateRequested = false;
				ResetEvent(mh_Alive);
				mh_Queue = CreateThread(NULL, 0, ProcessingThread, this, 0, &mn_QueueId);
			}

			return (mh_Queue == NULL) ? E_UNEXPECTED : S_OK;
		};
};
