#include "util.h"

OpenFileDialog::OpenFileDialog(void)
{
	this->DefaultExtension = 0;
	this->FileName = new TCHAR[MAX_PATH];
	this->Filter = 0;
	this->FilterIndex = 0;
	this->Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	this->InitialDir = 0;
	this->Owner = 0;
	this->Title = 0;
}

bool OpenFileDialog::ShowDialog()
{
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = this->Owner;
	ofn.lpstrDefExt = this->DefaultExtension;
	ofn.lpstrFile = this->FileName;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = this->Filter;
	ofn.nFilterIndex = this->FilterIndex;
	ofn.lpstrInitialDir = this->InitialDir;
	ofn.lpstrTitle = this->Title;
	ofn.Flags = this->Flags;

	GetOpenFileName(&ofn);

	if (_tcslen(this->FileName) == 0) return false;

	return true;
}

TCHAR *pszMonths[] = 
{
	_T("January"),
	_T("February"),
	_T("March"),
	_T("April"),
	_T("May"),
	_T("June"),
	_T("July"),
	_T("August"),
	_T("September"),
	_T("October"),
	_T("November"),
	_T("December")
};

TCHAR *pszDays[] = 
{
	_T("Sunday"),
	_T("Monday"),
	_T("Tuesday"),
	_T("Wednesday"),
	_T("Thursday"),
	_T("Friday"),
	_T("Saturday")
};

void GetLocalTimeString(TCHAR *pszOutTime)
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	_stprintf(pszOutTime, _T("%d / %d / %d, %02d:%02d:%02d"), st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond);
}

