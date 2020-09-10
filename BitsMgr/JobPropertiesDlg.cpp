#include "pch.h"
#include "JobPropertiesDlg.h"
#include "DialogHelper.h"
#include "View.h"

CJobPropertiesDlg::CJobPropertiesDlg(IBackgroundCopyJob* pJob) : m_pJob(pJob) {
}

void CJobPropertiesDlg::InitJob() {
    PWSTR name;
    m_pJob->GetDisplayName(&name);
    SetDlgItemText(IDC_DISPLAYNAME, name);
    ::CoTaskMemFree(name);

    m_pJob->GetDescription(&name);
    SetDlgItemText(IDC_DESC, name);
    ::CoTaskMemFree(name);

    GUID id;
    m_pJob->GetId(&id);
    WCHAR text[64];
    ::StringFromGUID2(id, text, _countof(text));
    SetDlgItemText(IDC_GUID, text);

    BG_JOB_STATE state;
    m_pJob->GetState(&state);
    SetDlgItemText(IDC_STATE, CView::JobStateToString(state));
    BG_JOB_PRIORITY priority;
    m_pJob->GetPriority(&priority);
    m_Priority.SelectString(-1, CView::JobPriorityToString(priority));

    CComPtr<IEnumBackgroundCopyFiles> spFiles;
    m_pJob->EnumFiles(&spFiles);
    if (spFiles) {
        ULONG count;
        spFiles->GetCount(&count);
        for (ULONG i = 0; i < count; i++) {
            CComPtr<IBackgroundCopyFile> spFile;
            spFiles->Next(1, &spFile, nullptr);
            if (spFile) {
                spFile->GetRemoteName(&name);
                int n = m_List.InsertItem(m_List.GetItemCount(), name);
                ::CoTaskMemFree(name);
                spFile->GetLocalName(&name);
                m_List.SetItemText(n, 1, name);
                ::CoTaskMemFree(name);
                if (state != BG_JOB_STATE_ERROR) {
                    BG_FILE_PROGRESS progress;
                    spFile->GetProgress(&progress);
                    CString text;
                    text.Format(L"%llu K / %llu B", progress.BytesTransferred >> 10, progress.BytesTotal);
                    m_List.SetItemText(n, 2, text);
                }
            }
        }
    }
}

LRESULT CJobPropertiesDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
    DialogHelper::AdjustOKCancelButtons(this);
    DialogHelper::SetDialogIcon(this, IDI_FILE);

    m_List.Attach(GetDlgItem(IDC_FILES));
    m_List.InsertColumn(0, L"Remote URL", LVCFMT_LEFT, 250);
    m_List.InsertColumn(1, L"Local Path", LVCFMT_LEFT, 250);
    m_List.InsertColumn(2, L"Progress", LVCFMT_CENTER, 100);

    BG_JOB_PROGRESS progress;
    m_pJob->GetProgress(&progress);
    SetDlgItemText(IDC_PROGRESS, CView::JobProgressToString(progress));

    m_Priority.Attach(GetDlgItem(IDC_PRIORITY));
    BG_JOB_PRIORITY priorities[] = { BG_JOB_PRIORITY_FOREGROUND, BG_JOB_PRIORITY_HIGH, BG_JOB_PRIORITY_NORMAL, BG_JOB_PRIORITY_LOW };
    for(decltype(priorities[0]) p : priorities)
        m_Priority.SetItemData(m_Priority.AddString(CView::JobPriorityToString(p)), p);

    InitJob();

    return 0;
}

LRESULT CJobPropertiesDlg::OnCloseCmd(WORD, WORD id, HWND, BOOL&) {
    if (id == IDOK) {
        // update job
        CString text;
        GetDlgItemText(IDC_DISPLAYNAME, text);
        m_pJob->SetDisplayName(text);
        GetDlgItemText(IDC_DESC, text);
        m_pJob->SetDescription(text);
        m_pJob->SetPriority((BG_JOB_PRIORITY)m_Priority.GetItemData(m_Priority.GetCurSel()));

    }
    EndDialog(id);
    return 0;
}
