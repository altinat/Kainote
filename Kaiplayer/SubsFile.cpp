﻿//  Copyright (c) 2016, Marcin Drob

//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  Kainote is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with Kainote.  If not, see <http://www.gnu.org/licenses/>.


#include "SubsFile.h"
#include "KaiListCtrl.h"
#include "MappedButton.h"

//pamiętaj ilość elementów tablicy musi być równ ilości enumów
wxString historyNames[] = {
	"",//pierwszy element którego nie używamy a musi być ostatni enum weszedł a także ochronić nas przed potencjalnym 0
	_("Otwarcie napisów"),
	_("Nowe napisy"),
	_("Edycja linii"),
	_("Edycja wielu linii"),
	_("Poprawa błędu pisowni w polu tekstowym"),
	_("Powielenie linijek"),
	_("Połączenie linijek"),
	_("Połączenie linijki z poprzednią"),
	_("Połączenie linijki z następną"),
	/*10*/_("Połączenie linijek pozostawienie pierszej"),
	_("Połączenie linijek pozostawienie ostatniej"),
	_("Wklejenie linijek"),
	_("Wklejenie kolumn"),
	_("Wklejenie tłumaczenia"),
	_("Przesunięcie tekstu tłumaczenia"),
	_("Ustawienie czasów linii jako ciągłych"),
	_("Ustawienie FPSu obliczonego z wideo"),
	_("Ustawienie własnego FPSu"),
	_("Zamiana linijek"),
	/*20*/_("Konwersja napisów"),
	_("Sortowanie napisów"),
	_("Usunięcie linijek"),
	_("Usunięcie tekstu"),
	_("Ustawienie czasu początkowego"),
	_("Ustawienie czasu końcowego"),
	_("Włączenie trybu tłumaczenia"),
	_("Wyłączenie trybu tłumaczenia"),
	_("Dodanie nowej linii"),
	_("Wstawienie linii"),
	/*30*/_("Zmiana czasu na wykresie audio"),
	_("Przyklejenie do klatki kluczowej"),
	_("Zmiana nagłówku napisów"),
	_("Akcja zaznacz linijki"),
	_("Przesunięcie czasów"),
	_("Poprawa błędu pisowni"),
	_("Edycja stylów"),
	_("Zmiana rozdzielczości napisów"),
	_("Narzędzie pozycjonowania"),
	_("Narzędzie ruchu"),
	/*40*/_("Narzędzie skalowania"),
	_("Narzędzie obrotów w osi Z"),
	_("Narzędzie obrotów w osiach X i Y"),
	_("Narzędzie wycinów prostokątnych"),
	_("Narzędzie wycinów wektorowych"),
	_("Narzędzie rysunków wektorowych"),
	_("Narzędzie zmieniacz pozycji"),
	_("Zamień"),
	_("Zamień wszystko"),
	_("Skrypt automatyzacji"),
};

HistoryDialog::HistoryDialog(wxWindow *parent, SubsFile *file, std::function<void(int)> func )
	: KaiDialog(parent, -1, _("Historia"),wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER)
{
	wxArrayString history;
	file->GetHistoryTable(&history);
	KaiListCtrl *HistoryList = new KaiListCtrl(this, ID_HISTORY_LIST, history);
	//HistoryList->ScrollTo(file->Iter()-2);
	Bind(LIST_ITEM_DOUBLECLICKED, [=](wxCommandEvent &evt){
		func(HistoryList->GetSelection());
	}, ID_HISTORY_LIST);
	DialogSizer *main = new DialogSizer(wxVERTICAL);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	MappedButton *Set = new MappedButton(this, ID_SET_HISTORY, _("Ustaw"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		func(HistoryList->GetSelection());
	}, ID_SET_HISTORY);
	MappedButton *Ok = new MappedButton(this, ID_SET_HISTORY_AND_CLOSE, _("OK"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		func(HistoryList->GetSelection());
		Hide();
	}, ID_SET_HISTORY_AND_CLOSE);
	MappedButton *Cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
	buttonSizer->Add(Set,1,wxALL,3);
	buttonSizer->Add(Ok,1,wxALL,3);
	buttonSizer->Add(Cancel,1,wxALL,3);
	main->Add(HistoryList,1,wxEXPAND|wxALL,3);
	main->Add(buttonSizer,0, wxCENTER);
	main->SetMinSize(300,400);
	SetSizerAndFit(main);
	CenterOnParent();
	HistoryList->SetSelection(file->Iter(),true);
}


File::File()
	:etidtionType(0)
	,activeLine(0)
{
}

File::~File()
{
	dials.clear();
	styles.clear();
	sinfo.clear();
	ddials.clear();
	dstyles.clear();
	dsinfo.clear();
	sel.clear();
	//wxLogStatus("Clearing");
}
void File::Clear()
{
	
	for(std::vector<Dialogue*>::iterator it = ddials.begin(); it != ddials.end(); it++)
	{
		
		delete (*it);
		
	}
	
	for(std::vector<Styles*>::iterator it = dstyles.begin(); it != dstyles.end(); it++)
	{
		delete (*it);
		
	}
	
	for(std::vector<SInfo*>::iterator it = dsinfo.begin(); it != dsinfo.end(); it++)
	{
		delete (*it);
		
	}
}



File *File::Copy()
{
	File *file=new File();
	file->dials = dials;
	file->styles= styles;
	file->sinfo = sinfo;
	file->sel = sel;
	file->activeLine = activeLine;
	return file;
}

SubsFile::SubsFile()
{
    iter=0;
	edited=false;
	subs = new File();
	IdConverter = new AVLtree();
}

SubsFile::~SubsFile()
{
	subs->Clear();
	delete subs;
	for(std::vector<File*>::iterator it = undo.begin(); it != undo.end(); it++)
	{
		(*it)->Clear();
		delete (*it);
	}
    undo.clear();
	delete IdConverter;
}


void SubsFile::SaveUndo(unsigned char editionType, int activeLine)
{
	int size=maxx();
	if(iter!=size){
		for(std::vector<File*>::iterator it = undo.begin()+iter+1; it != undo.end(); it++)
		{
			(*it)->Clear();
			delete (*it);
		}
		undo.erase(undo.begin()+iter+1, undo.end());
	}
	subs->activeLine = activeLine;
	subs->etidtionType = editionType;
	undo.push_back(subs);
	subs=subs->Copy();
	iter++;
	edited=false;
}


void SubsFile::Redo()
{
    if(iter<maxx()){
		iter++;
		subs->Clear();
		delete subs;
		subs=undo[iter]->Copy();
		ReloadVisibleDialogues();
	}
}

void SubsFile::Undo()
{
    if(iter>0){
		iter--;
		subs->Clear();
		delete subs;
		subs=undo[iter]->Copy();
		ReloadVisibleDialogues();
	}
}

bool SubsFile::SetHistory(int _iter)
{
    if(_iter < undo.size() && _iter>=0){
		iter = _iter;
		subs->Clear();
		delete subs;
		subs=undo[iter]->Copy();
		ReloadVisibleDialogues();
		return false;
	}
	return true;
}

void SubsFile::DummyUndo()
{
	subs->Clear();
	delete subs;
	subs=undo[iter]->Copy();
	ReloadVisibleDialogues();
}

void SubsFile::DummyUndo(int newIter)
{
	if(newIter < 0 || newIter >= undo.size()){return;}
	subs->Clear();
	delete subs;
	subs=undo[newIter]->Copy();
	ReloadVisibleDialogues();
	iter = newIter;
	if(iter < undo.size() - 1){
		for(std::vector<File*>::iterator it = undo.begin()+iter+1; it != undo.end(); it++)
		{
			(*it)->Clear();
			delete (*it);
		}
		undo.erase(undo.begin()+iter+1, undo.end());
	}
}

bool SubsFile::IsNotSaved()
{
    if((subs->ddials.size()==0 && subs->dstyles.size()==0 && subs->dsinfo.size()==0 && !edited)){return false;}
    return true;
}

int SubsFile::maxx()
{
    return undo.size()-1;
}

int SubsFile::Iter()
{
    return iter;
}

Dialogue *SubsFile::CopyDial(int i, bool push, bool keepstate)
{
	Dialogue *dial= GetDialogue(i)->Copy(keepstate);
	subs->ddials.push_back(dial);
	if (push){ (*this)[i] = dial; }
	return dial;
}

Dialogue *SubsFile::GetDialogue(int i)
{
	int Id = (*IdConverter)[i];
	if (Id < 0){
		Id = (*IdConverter)[IdConverter->size() - 1];
		wxLogStatus("przekroczone drzewko %i, %i", i, IdConverter->size());
	}
	return subs->dials[Id];
}

Dialogue *&SubsFile::operator[](int i)
{
	int Id = (*IdConverter)[i];
	if (Id < 0){
		Id = (*IdConverter)[IdConverter->size() - 1];
		wxLogStatus("przekroczone drzewko %i, %i", i, IdConverter->size());
	}
	return subs->dials[Id];
}

void SubsFile::DeleteDialogues(int from, int to)
{
	edited = true;
	subs->dials.erase(subs->dials.begin() + IdConverter->getElementById(from), subs->dials.begin() + IdConverter->getElementById(to));
	for (int i = from; i <= to; i++){
		IdConverter->deleteItemById(i);
	}
}
	
Styles *SubsFile::CopyStyle(int i, bool push)
{
	Styles *styl=subs->styles[i]->Copy();
	subs->dstyles.push_back(styl);
	if(push){subs->styles[i]=styl;}
	return styl;
}
	
SInfo *SubsFile::CopySinfo(int i, bool push)
{
	SInfo *sinf=subs->sinfo[i]->Copy();
	subs->dsinfo.push_back(sinf);
	if(push){subs->sinfo[i]=sinf;}
	return sinf;
}

void SubsFile::EndLoad(unsigned char editionType, int activeLine)
{
	subs->activeLine = activeLine;
	subs->etidtionType = editionType;
	undo.push_back(subs);
	subs=subs->Copy();
	//LoadVisibleDialogues();
}

void SubsFile::RemoveFirst(int num)
{
	//uwaga pierwszym elementem tablicy są napisy zaraz po wczytaniu dlatego też nie należy go usuwać
	for(std::vector<File*>::iterator it = undo.begin()+1; it != undo.begin()+num; it++)
	{
		(*it)->Clear();
		delete (*it);
	}
	undo.erase(undo.begin()+1, undo.begin()+num);
	iter-=(num-1);
}

void SubsFile::GetURStatus(bool *_undo, bool *_redo)
{
	*_redo= (iter<(int)undo.size()-1);
	*_undo= (iter>0);
}

File *SubsFile::GetSubs()
{
	return subs;
}

void SubsFile::LoadVisibleDialogues()
{
	for (size_t i = 0; i < subs->dials.size();i++){
		if (subs->dials[i]->isVisible){
			IdConverter->insert(i);
		}
	}
}

void SubsFile::ReloadVisibleDialogues()
{
	for (size_t i = 0; i < subs->dials.size(); i++){
		bool visible = subs->dials[i]->isVisible;
		if (visible && IdConverter->getElementByKey(i) == -1){
			IdConverter->insert(i);
		}
		else if (!visible && IdConverter->getElementByKey(i) != -1){
			IdConverter->deleteItemByKey(i);
		}
	}
	int lastElementKey = IdConverter->getElementById(IdConverter->size() - 1);
	if (lastElementKey >= subs->dials.size()){
		for (int i = subs->dials.size(); i <= lastElementKey; i++)
			IdConverter->deleteItemByKey(i);
	}
}

void SubsFile::GetHistoryTable(wxArrayString *history)
{
	for(size_t i = 0; i < undo.size(); i++){
		history->push_back(historyNames[undo[i]->etidtionType] + 
			wxString::Format(_(", aktywna linia %i"), undo[i]->activeLine));
	}
}

void SubsFile::ShowHistory(wxWindow *parent, std::function<void(int)> functionAfterChangeHistory)
{
	HistoryDialog HD(parent, this, functionAfterChangeHistory);
	HD.ShowModal();
}