//////////////////////////////////////////////////////////////////////////////
// M.A.X. - dialog.h
//////////////////////////////////////////////////////////////////////////////
#ifndef dialogH
#define dialogH
#include "defines.h"
#include "main.h"

void ShowDialog(string text,bool pure,string path,int SaveLoad=-1);
void ShowDialogList(TList *list,int offset);
bool ShowYesNo(string text);
int ShowNumberInput(string text);
void ShowOK(string text,bool pure=false);

#endif
