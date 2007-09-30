//////////////////////////////////////////////////////////////////////////////
// M.A.X. - dialog.h
//////////////////////////////////////////////////////////////////////////////
#ifndef dialogH
#define dialogH
#include "defines.h"
#include "main.h"

void ShowDialog(string text,bool pure,string path,int SaveLoad=-1);
void ShowDialogList(TList *list,int offset);
int __fastcall SortFileList(void * Item1, void * Item2);
bool ShowYesNo(string text);
int ShowNumberInput(string text);
void ShowOK(string text,bool pure=false);

#endif
