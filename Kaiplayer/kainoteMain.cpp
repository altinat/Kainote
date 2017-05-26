﻿/***************************************************************
* Name:      kainoteMain.cpp
* Purpose:   Code for Application Frame
* Author:    Bjakja (bjakja@op.pl)
* Created:   2012-04-23
* Copyright: Marcin Drob aka Bjakja (http://animesub.info/forum/viewtopic.php?id=258715)
* License:   
* Kainote is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* Kainote is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with Kainote.  If not, see <http://www.gnu.org/licenses/>.

**************************************************************/


#include "KainoteMain.h"
#include "SubsTime.h"
#include "ScriptInfo.h"
#include "Config.h"
#include "OptionsDialog.h"
#include "DropFiles.h"
#include "OpennWrite.h"
#include "Hotkeys.h"
#include "FontCollector.h"
#include "Menu.h"
#include <wx/accel.h>
#include <wx/dir.h>
#include <wx/sysopt.h>
#include "KaiTextCtrl.h"
#include "KaiMessageBox.h"
#include "FontEnumerator.h"

#undef IsMaximized
#if _DEBUG
#define logging 5
#endif
//#define wxIMAGE_PNG(x) wxImage(wxS(#x),wxBITMAP_TYPE_PNG_RESOURCE)


kainoteFrame::kainoteFrame(const wxPoint &pos, const wxSize &size)
	: KaiFrame(0, -1, _("Bez nazwy - ")+Options.progname, pos, size, wxDEFAULT_FRAME_STYLE)
	,badResolution(false)
	,fc(NULL)
{

#if logging
	mylog=new wxLogWindow(this, "Logi",true, false);
	mylog->PassMessages(true);
#else
	mylog=NULL;
#endif
	FR=NULL;
	SL=NULL;
	Auto=NULL;
	Options.GetTable(SubsRecent, subsrec);
	Options.GetTable(VideoRecent, videorec);
	Options.GetTable(AudioRecent, audsrec);
	wxFont thisFont(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma",wxFONTENCODING_DEFAULT);
	SetFont(thisFont);
	wxIcon kaiicon("aaaa",wxBITMAP_TYPE_ICO_RESOURCE); 
	SetIcon(kaiicon);
	
	//height 22 zmieniając jedną z tych wartości popraw je też dropfiles
	Menubar = new MenuBar(this);

	//FrameSizer *mains1= new FrameSizer(wxVERTICAL);
	//wxBoxSizer *mains1= new wxBoxSizer(wxVERTICAL);
	//mains=new wxBoxSizer(wxHORIZONTAL);
	Tabs=new Notebook (this,ID_TABS);
	SetMinSize(wxSize(500,300));
	Toolbar=new KaiToolbar(this,Menubar,-1,true);

	//height 26 zmieniając jedną z tych wartości popraw je też dropfiles
	StatusBar = new KaiStatusBar(this, ID_STATUSBAR1);
	int StatusBarWidths[9] = { -12, 0, 0, 0, 0, 0, 0, 0, -22};
	StatusBar->SetFieldsCount(9,StatusBarWidths);
	wxString tooltips[]={"",_("Skala obrazu wideo"),_("Powiększenie wideo"),_("Czas trwania wideo"),
		_("FPS"),_("Rozdzielczość wideo"),_("Proporcje wideo"),_("Rozdzielczość napisów"),_("Nazwa pliku wideo")};
	StatusBar->SetTooltips(tooltips,9);
	//StatusBar->SetLabelBackgroundColour(2,"#FF0000");
	//StatusBar->SetLabelTextColour(2,"#000000");

	/*mains->Add(Toolbar,0,wxEXPAND,0);
	mains->Add(Tabs,1,wxEXPAND,0);
	mains1->Add(Menubar,0,wxEXPAND,0);
	mains1->Add(mains,1,wxEXPAND,0);
	mains1->Add(StatusBar,0,wxEXPAND,0);*/

	FileMenu = new Menu();
	FileMenu->AppendTool(Toolbar,OpenSubs, _("&Otwórz napisy"), _("Otwórz plik napisów"),PTR_BITMAP_PNG("opensubs"));
	FileMenu->AppendTool(Toolbar,SaveSubs, _("&Zapisz"), _("Zapisz aktualny plik"),PTR_BITMAP_PNG("save"));
	FileMenu->AppendTool(Toolbar,SaveAllSubs, _("Zapisz &wszystko"), _("Zapisz wszystkie napisy"),PTR_BITMAP_PNG("saveall"));
	FileMenu->AppendTool(Toolbar,SaveSubsAs, _("Zapisz &jako..."), _("Zapisz jako"),PTR_BITMAP_PNG("saveas"));
	FileMenu->AppendTool(Toolbar,SaveTranslation, _("Zapisz &tłumaczenie"), _("Zapisz tłumaczenie"),PTR_BITMAP_PNG("savetl"),false);
	SubsRecMenu = new Menu();
	//AppendRecent();
	FileMenu->AppendTool(Toolbar,RecentSubs, _("Ostatnio otwa&rte napisy"), _("Ostatnio otwarte napisy"),PTR_BITMAP_PNG("recentsubs"),true, SubsRecMenu);
	FileMenu->AppendTool(Toolbar,RemoveSubs, _("Usuń napisy z e&dytora"), _("Usuń napisy z edytora"),PTR_BITMAP_PNG("close"));
	FileMenu->Append(9989,_("Pokaż / Ukryj okno logów"));
	FileMenu->AppendTool(Toolbar,Settings, _("&Ustawienia"), _("Ustawienia programu"),PTR_BITMAP_PNG("SETTINGS"));
	FileMenu->AppendTool(Toolbar,Quit, _("Wyjści&e\tAlt-F4"), _("Zakończ działanie programu"),PTR_BITMAP_PNG("exit"));
	Menubar->Append(FileMenu, _("&Plik"));

	EditMenu = new Menu();
	EditMenu->AppendTool(Toolbar, Undo, _("&Cofnij"), _("Cofnij"),PTR_BITMAP_PNG("undo"),false);
	EditMenu->AppendTool(Toolbar, Redo, _("&Ponów"), _("Ponów"),PTR_BITMAP_PNG("redo"),false);
	EditMenu->AppendTool(Toolbar,FindReplace, _("Znajdź i za&mień"), _("Szuka i podmienia dane frazy tekstu"),PTR_BITMAP_PNG("findreplace"));
	EditMenu->AppendTool(Toolbar,Search, _("Z&najdź"), _("Szuka dane frazy tekstu"),PTR_BITMAP_PNG("search"));
	Menu *SortMenu[2];
	for(int i=0; i<2; i++){
		SortMenu[i]=new Menu();
		SortMenu[i]->Append(7000+(6*i),_("Czas początkowy"),_("Sortuj według czasu początkowego"));
		SortMenu[i]->Append(7001+(6*i),_("Czas końcowy"),_("Sortuj według czasu końcowego"));
		SortMenu[i]->Append(7002+(6*i),_("Style"),_("Sortuj według styli"));
		SortMenu[i]->Append(7003+(6*i),_("Aktor"),_("Sortuj według aktora"));
		SortMenu[i]->Append(7004+(6*i),_("Efekt"),_("Sortuj według efektu"));
		SortMenu[i]->Append(7005+(6*i),_("Warstwa"),_("Sortuj według warstwy"));
	}

	EditMenu->AppendTool(Toolbar,SortLines, _("Sort&uj wszystkie linie"), _("Sortuje wszystkie linie napisów ASS"),PTR_BITMAP_PNG("sort"),true,SortMenu[0]);
	EditMenu->AppendTool(Toolbar,SortSelected, _("Sortu&j zaznaczone linie"),_("Sortuje zaznaczone linie napisów ASS"),PTR_BITMAP_PNG("sortsel"),true, SortMenu[1]);
	EditMenu->AppendTool(Toolbar,SelectLines, _("Zaznacz &linijki"), _("Zaznacza linijki wg danej frazy tekstu"),PTR_BITMAP_PNG("sellines"));
	Menubar->Append(EditMenu, _("&Edycja"));

	VidMenu = new Menu();
	VidMenu->AppendTool(Toolbar,OpenVideo, _("Otwórz wideo"), _("Otwiera wybrane wideo"), PTR_BITMAP_PNG("openvideo"));
	VidsRecMenu = new Menu();
	VidMenu->AppendTool(Toolbar, RecentVideo, _("Ostatnio otwarte wideo"), _("Ostatnio otwarte video"),PTR_BITMAP_PNG("recentvideo"),true, VidsRecMenu);
	VidMenu->AppendTool(Toolbar, SetStartTime, _("Wstaw czas początkowy z wideo"), _("Wstawia czas początkowy z wideo"),PTR_BITMAP_PNG("setstarttime"),false);
	VidMenu->AppendTool(Toolbar, SetEndTime, _("Wstaw czas końcowy z wideo"), _("Wstawia czas końcowy z wideo"),PTR_BITMAP_PNG("setendtime"),false);
	VidMenu->AppendTool(Toolbar, PreviousFrame, _("Klatka w tył"), _("Przechodzi o jedną klatkę w tył"),PTR_BITMAP_PNG("prevframe"),false);
	VidMenu->AppendTool(Toolbar, NextFrame, _("Klatka w przód"), _("Przechodzi o jedną klatkę w przód"),PTR_BITMAP_PNG("nextframe"),false);
	VidMenu->AppendTool(Toolbar, SetVideoAtStart,_("Przejdź do czasu początkowego linii"),_("Przechodzi wideo do czasu początkowego linii"),PTR_BITMAP_PNG("videoonstime"));
	VidMenu->AppendTool(Toolbar, SetVideoAtEnd,_("Przejdź do czasu końcowego linii"),_("Przechodzi wideo do czasu końcowego linii"),PTR_BITMAP_PNG("videoonetime"));
	VidMenu->AppendTool(Toolbar, PlayPauseG, _("Odtwarzaj / Pauza"), _("Odtwarza lub pauzuje wideo"),PTR_BITMAP_PNG("pausemenu"),false);
	VidMenu->AppendTool(Toolbar, GoToPrewKeyframe,_("Przejdź do poprzedniej klatki kluczowej"),"",PTR_BITMAP_PNG("prevkeyframe"));
	VidMenu->AppendTool(Toolbar, GoToNextKeyframe,_("Przejdź do następnej klatki kluczowej"),"",PTR_BITMAP_PNG("nextkeyframe"));
	VidMenu->AppendTool(Toolbar, VideoZoom, _("Powiększ wideo"), "", PTR_BITMAP_PNG("zoom"));
	VidMenu->Append(VideoIndexing, _("Otwieraj wideo przez FFMS2"), _("Otwiera wideo przez FFMS2, co daje dokładność klatkową"),true,0,0,ITEM_CHECK)->Check(Options.GetBool(VideoIndex));

	Menubar->Append(VidMenu, _("&Wideo"));

	AudMenu = new Menu();
	AudMenu->AppendTool(Toolbar,OpenAudio, _("Otwórz audio"), _("Otwiera wybrane audio"),PTR_BITMAP_PNG("openaudio"));
	AudsRecMenu = new Menu();

	AudMenu->AppendTool(Toolbar,RecentAudio, _("Ostatnio otwarte audio"), _("Ostatnio otwarte audio"),PTR_BITMAP_PNG("recentaudio"),true , AudsRecMenu);
	AudMenu->AppendTool(Toolbar,AudioFromVideo, _("Otwórz audio z wideo"), _("Otwiera audio z wideo"),PTR_BITMAP_PNG("audiofromvideo"));
	AudMenu->AppendTool(Toolbar,CloseAudio, _("Zamknij audio"), _("Zamyka audio"),PTR_BITMAP_PNG("closeaudio"));
	Menubar->Append(AudMenu, _("A&udio"));

	ViewMenu = new Menu();
	ViewMenu->Append(ViewAll, _("Wszystko"), _("Wszystkie okna są widoczne"));
	ViewMenu->Append(ViewVideo, _("Wideo i napisy"), _("Widoczne tylko okno wideo i napisów"));
	ViewMenu->Append(ViewAudio, _("Audio i napisy"), _("Widoczne tylko okno audio i napisów"));
	ViewMenu->Append(ViewSubs, _("Tylko napisy"), _("Widoczne tylko okno napisów"));
	Menubar->Append(ViewMenu, _("Wido&k"));

	SubsMenu = new Menu();
	SubsMenu->AppendTool(Toolbar,Editor, _("Włącz / Wyłącz edytor"), _("Włączanie bądź wyłączanie edytora"),PTR_BITMAP_PNG("editor"));
	SubsMenu->AppendTool(Toolbar,ASSProperties, _("Właściwości ASS"), _("Właściwości napisów ASS"),PTR_BITMAP_PNG("ASSPROPS"));
	SubsMenu->AppendTool(Toolbar,StyleManager, _("&Menedżer stylów"), _("Służy do zarządzania stylami ASS"),PTR_BITMAP_PNG("styles"));
	ConvMenu = new Menu();
	ConvMenu->AppendTool(Toolbar,ConvertToASS, _("Konwertuj do ASS"), _("Konwertuje do formatu ASS"),PTR_BITMAP_PNG("convass"),false);
	ConvMenu->AppendTool(Toolbar,ConvertToSRT, _("Konwertuj do SRT"), _("Konwertuje do formatu SRT"),PTR_BITMAP_PNG("convsrt"));
	ConvMenu->AppendTool(Toolbar,ConvertToMDVD, _("Konwertuj do MDVD"), _("Konwertuje do formatu microDVD"),PTR_BITMAP_PNG("convmdvd"));
	ConvMenu->AppendTool(Toolbar,ConvertToMPL2, _("Konwertuj do MPL2"), _("Konwertuje do formatu MPL2"),PTR_BITMAP_PNG("convmpl2"));
	ConvMenu->AppendTool(Toolbar,ConvertToTMP, _("Konwertuj do TMP"), _("Konwertuje do formatu TMPlayer (niezalecene)"),PTR_BITMAP_PNG("convtmp"));

	SubsMenu->Append(ID_CONV, _("Konwersja"), _("Konwersja z jednego formatu napisów na inny"),true, PTR_BITMAP_PNG("convert"), ConvMenu);
	SubsMenu->AppendTool(Toolbar,ChangeTime, _("Okno zmiany &czasów\tCtrl-I"), _("Przesuwanie czasów napisów"),PTR_BITMAP_PNG("times"));
	SubsMenu->AppendTool(Toolbar,FontCollectorID, _("Kolekcjoner czcionek"), _("Kolekcjoner czcionek"),PTR_BITMAP_PNG("fontcollector"));
	SubsMenu->AppendTool(Toolbar,HideTags, _("Ukryj tagi w nawiasach"), _("Ukrywa tagi w nawiasach ASS i MDVD"),PTR_BITMAP_PNG("hidetags"));
	Menubar->Append(SubsMenu, _("&Napisy"));

	AutoMenu = new Menu();
	AutoMenu->AppendTool(Toolbar,AutoLoadScript, _("Wczytaj skrypt"), _("Wczytaj skrypt"),PTR_BITMAP_PNG("automation"));
	AutoMenu->AppendTool(Toolbar,AutoReloadAutoload, _("Odśwież skrypty autoload"), _("Odśwież skrypty autoload"),PTR_BITMAP_PNG("automation"));
	AutoMenu->Append(LoadLastScript, _("Uruchom ostatnio zaczytany skrypt"), _("Uruchom ostatnio zaczytany skrypt"));
	Menubar->Append(AutoMenu, _("Au&tomatyzacja"));

	HelpMenu = new Menu();
	HelpMenu->AppendTool(Toolbar,Help, _("&Pomoc (niekompletna, ale jednak)"), _("Otwiera pomoc w domyślnej przeglądarce"),PTR_BITMAP_PNG("help"));
	HelpMenu->AppendTool(Toolbar,ANSI, _("&Wątek programu na forum AnimeSub.info"), _("Otwiera wątek programu na forum AnimeSub.info"),PTR_BITMAP_PNG("ansi"));
	HelpMenu->AppendTool(Toolbar,About, _("&O programie"), _("Wyświetla informacje o programie"),PTR_BITMAP_PNG("about"));
	HelpMenu->AppendTool(Toolbar,Helpers, _("&Lista osób pomocnych przy tworzeniu programu"), _("Wyświetla listę osób pomocnych przy tworzeniu programu"),PTR_BITMAP_PNG("helpers"));
	Menubar->Append(HelpMenu, _("Pomo&c"));

	Toolbar->InitToolbar();

	//SetSizer(mains1);

	SetAccels(false);


	Connect(ID_TABS,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnPageChanged,0,this);
	Connect(ID_ADDPAGE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnPageAdd);
	Connect(ID_CLOSEPAGE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnPageClose);
	Connect(NextTab,PreviousTab,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnPageChange);
	//tutaj dodawaj nowe idy
	Connect(SaveSubs,Undo,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnMenuSelected);
	Connect(7000,7011,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnMenuSelected);
	Connect(ID_MOVE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnMenuSelected);
	Connect(OpenSubs,ANSI,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnMenuSelected1);
	Connect(SelectFromVideo,PlayActualLine,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnMenuSelected1);
	Connect(Plus5SecondG,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnP5Sec);
	Connect(Minus5SecondG,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnM5Sec);
	Connect(PreviousLine,JoinWithNext,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnChangeLine);
	Connect(Remove,RemoveText,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnDelete);
	Menubar->Connect(EVT_MENU_OPENED,(wxObjectEventFunction)&kainoteFrame::OnMenuOpened,0,this);
	Connect(wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&kainoteFrame::OnClose1);
	Connect(30000,30059,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnRecent);
	Connect(PlayActualLine,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&kainoteFrame::OnMenuSelected1);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &event){
		if(!mylog){
			mylog=new wxLogWindow(this, "Logi",true, false);
			mylog->PassMessages(true);
		}else{
			delete mylog; mylog=NULL;
		}
		
	},9989);
	Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent &event){
		TabPanel *tab = GetTab();
		if(tab->Grid1->IsShown()){tab->Grid1->SetFocus();}else{tab->Video->SetFocus();}
	});
	Connect(SnapWithStart,SnapWithEnd,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&kainoteFrame::OnAudioSnap);
	SetDropTarget(new DragnDrop(this));
	Bind(wxEVT_SIZE,&kainoteFrame::OnSize,this);


	bool im=Options.GetBool(WindowMaximized);
	if(im){Maximize(Options.GetBool(WindowMaximized));}

	if(!Options.GetBool(EditorOn)){HideEditor();}	
	std::set_new_handler(OnOutofMemory);
	FontEnum.StartListening(this);
	SetSubsResolution(false);
	Auto=new Auto::Automation();
}

kainoteFrame::~kainoteFrame()
{

	bool im=IsMaximized();
	if(!im && !IsIconized()){
		int posx,posy,sizex,sizey;
		GetPosition(&posx,&posy);
		if(posx<2000){
			Options.SetCoords(WindowPosition,posx,posy);}
		GetSize(&sizex,&sizey);
		Options.SetCoords(WindowSize,sizex,sizey);
	}

	Toolbar->Destroy();
	Options.SetBool(WindowMaximized,im);
	Options.SetBool(EditorOn,GetTab()->edytor);
	Options.SetTable(SubsRecent,subsrec);
	Options.SetTable(VideoRecent,videorec);
	Options.SetTable(AudioRecent,audsrec);
	Options.SetInt(VideoVolume,GetTab()->Video->volslider->GetValue());

	Options.SaveOptions();

	StyleStore::DestroyStore();
	if(FR){FR->Destroy();FR=NULL;}
	if(Auto){delete Auto; Auto=NULL;}
	if(fc){delete fc; fc=NULL;}
	SpellChecker::Destroy();
	VideoToolbar::DestroyIcons();
}


//elementy menu które ulegają wyłączaniu
void kainoteFrame::OnMenuSelected(wxCommandEvent& event)
{
	int id=event.GetId();
	int Modif = event.GetInt();

	TabPanel *tab=GetTab();
	//wxLogStatus("menu %i %i", id, Modif);
	if(Modif == wxMOD_SHIFT){
		MenuItem *item=Menubar->FindItem(id);
		//upewnij się, że da się zmienić idy na nazwy, 
		//może i trochę spowolni operację ale skończy się ciągłe wywalanie hotkeysów
		//może od razu funkcji onmaphotkey przekazać item by zrobiła co trzeba
		int ret=-1;
		wxString name=item->GetLabelText();
		ret=Hkeys.OnMapHkey( id, name, this, GLOBAL_HOTKEY);
		if(ret > -2){
			idAndType itype(id);
			item->SetAccel(&Hkeys.GetHKey(itype));
			SetAccels(false); 
			Hkeys.SaveHkeys();
		}
		return;
	}
	

	if(id==SaveSubs){
		Save(false);
	}else if(id==SaveSubsAs){
		Save(true);
	}else if(id==SaveAllSubs){
		SaveAll();
	}else if(id==SaveTranslation){
		GetTab()->Grid1->AddSInfo("TLMode", "Translated",false);
		Save(true, -1, false);
		GetTab()->Grid1->AddSInfo("TLMode", "Yes",false);
	}else if(id==RemoveSubs){
		if(SavePrompt(3)){event.SetInt(-1);return;}
		if(tab->SubsPath!=("")){
			tab->SubsName=_("Bez tytułu");
			tab->SubsPath="";
			Label();
			tab->Grid1->Clearing();
			tab->Grid1->file=new SubsFile();
			tab->Grid1->LoadDefault();
			tab->Edit->RefreshStyle(true);
			tab->Grid1->RepaintWindow();

			if(tab->Video->GetState()!=None){tab->Video->OpenSubs(NULL);tab->Video->Render();}
		}
	}else if(id==Undo){
		tab->Grid1->GetUndo(false);
	}else if(id==Redo){
		tab->Grid1->GetUndo(true);
	}else if(id==Search || id==FindReplace){
		if(FR && FR->IsShown() && FR->repl && id==FindReplace){FR->Hide();return;}
		FR= new findreplace(this, FR, id==FindReplace);
		FR->Show(true);
		FR->ReloadStyle();
		wxActivateEvent evt;
		FR->OnSetFocus(evt);
	}else if(id==SelectLines){
		SL= new findreplace(this,SL,false,true);
		SL->Show(true);
	}else if(id==PlayPauseG){
		tab->Video->Pause();
	}else if(id==PreviousFrame||id==NextFrame){
		tab->Video->MovePos((id==PreviousFrame)? -1 : 1);
	}else if(id==SetStartTime||id==SetEndTime){
		if(tab->Video->GetState()!=None){
			if(id==SetStartTime){
				int time = tab->Video->GetFrameTime() + Options.GetInt(InsertStartOffset);
				tab->Grid1->SetStartTime(ZEROIT(time));
			}else{
				int time = tab->Video->GetFrameTime(false)+Options.GetInt(InsertEndOffset);
				tab->Grid1->SetEndTime(ZEROIT(time));
			}
		}
	}else if(id==VideoIndexing){
		MenuItem *Item=Menubar->FindItem(VideoIndexing);
		Options.SetBool(VideoIndex,Item->IsChecked());
	}else if(id==VideoZoom){
		tab->Video->SetZoom();
	}else if(id>=OpenAudio&&id<=CloseAudio){
		OnOpenAudio(event);
	}else if(id==ASSProperties){
		OnAssProps();
	}else if(id==StyleManager){
		StyleStore::ShowStore();
	}else if(id==FontCollectorID && tab->Grid1->form<SRT){
		if(!fc){fc=new FontCollector(this);}
		fc->ShowDialog(this);
	}else if(id>=ConvertToASS && id<=ConvertToMPL2){
		if(tab->Grid1->GetSInfo("TLMode")!="Yes"){
			OnConversion(( id - ConvertToASS ) + 1);
		}
	}else if(id==HideTags){
		tab->Grid1->HideOver();
	}else if(id==ChangeTime){
		bool show=!tab->CTime->IsShown();
		Options.SetBool(MoveTimesOn,show);
		tab->CTime->Show(show);
		tab->BoxSizer1->Layout();
	}else if(id>6999&&id<7012){
		bool all=id<7006;
		int difid=(all)? 7000 : 7006;
		tab->Grid1->SortIt(id-difid,all);
	}else if(id>=ViewAll&&id<=ViewSubs){
		bool vidshow=( id==ViewAll || id==ViewVideo ) && tab->Video->GetState()!=None;
		bool vidvis=tab->Video->IsShown();
		if(!vidshow && tab->Video->GetState()==Playing){tab->Video->Pause();}
		tab->Video->Show(vidshow);
		if(vidshow && !vidvis){
			tab->Edit->OnVideo=true;
			tab->Video->OpenSubs(tab->Grid1->GetVisible()/*SaveText()*/);
		}
		if(tab->Edit->ABox){
			tab->Edit->ABox->Show((id==ViewAll||id==ViewAudio));
			if(id==ViewAudio){tab->Edit->SetMinSize(wxSize(500,350));}
		}	
		tab->Layout();
	}else if(id==AutoLoadScript){
		if(!Auto){Auto=new Auto::Automation();}
		wxFileDialog *FileDialog1 = new wxFileDialog(this, _("Wybierz sktypt"), 
			Options.GetString(AutomationRecent),
			"", _("Pliki skryptów (*.lua),(*.moon)|*.lua;*.moon;"), wxFD_OPEN);
		if (FileDialog1->ShowModal() == wxID_OK){
			wxString file=FileDialog1->GetPath();
			Options.SetString(AutomationRecent,file.AfterLast('\\'));
			//if(Auto->Add(file)){Auto->BuildMenu(&AutoMenu);}
			Auto->Add(file);
		}
		FileDialog1->Destroy();

	}else if(id==AutoReloadAutoload){
		if(!Auto){Auto=new Auto::Automation();}
		else{Auto->ReloadScripts();}
		//Auto->BuildMenu(&AutoMenu);
	}else if(id ==LoadLastScript){
		if(!Auto){Auto=new Auto::Automation(true);}
		Auto->AddFromSubs();
		int size = Auto->ASSScripts.size();
		if(!size){KaiMessageBox(_("Ten plik napisów nie ma dodanych żadnych skryptów"));return;}
		auto script = Auto->ASSScripts[size-1];
		if(script->CheckLastModified(true)){script->Reload();}
		auto macro = script->GetMacro(0);
		if(macro){
			if(macro->Validate(tab)){
				macro->Run(tab);
			}else{
				KaiMessageBox(wxString::Format(_("Warunki skryptu Lua '%s' nie zostały spełnione"), script->GetPrettyFilename()),_("Błąd"));
			}
		}else{
			KaiMessageBox(wxString::Format(_("Błąd wczytywania skryptu Lua: %s\n%s"), script->GetPrettyFilename(), script->GetDescription()),_("Błąd"));
			Auto->OnEdit(script->GetFilename());	
		}
	}else if(id==GoToPrewKeyframe){
		tab->Video->GoToPrevKeyframe();
		tab->Video->RefreshTime();
	}else if(id==GoToNextKeyframe){
		tab->Video->GoToNextKeyframe();
		tab->Video->RefreshTime();
	}else if(id==SetVideoAtStart){
		int fsel=tab->Grid1->FirstSel();
		if(fsel<0){return;}
		tab->Video->Seek(MAX(0,tab->Grid1->GetDial(fsel)->Start.mstime),true);
	}else if(id==SetVideoAtEnd){
		int fsel=tab->Grid1->FirstSel();
		if(fsel<0){return;}
		tab->Video->Seek(MAX(0,tab->Grid1->GetDial(fsel)->End.mstime),false);
	}else if(id==ID_MOVE){
		tab->CTime->OnOKClick(event);
	}


}
//Stałe elementy menu które nie ulegają wyłączaniu
void kainoteFrame::OnMenuSelected1(wxCommandEvent& event)
{
	int id=event.GetId();
	int Modif = event.GetInt();
	TabPanel *pan=GetTab();
	
	if(Modif==wxMOD_SHIFT){
		MenuItem *item=Menubar->FindItem(id);
		int ret=-1;
		wxString name=item->GetLabelText();
		ret=Hkeys.OnMapHkey( id, name, this, GLOBAL_HOTKEY);
		if(ret>-2){
			idAndType itype(id);
			item->SetAccel(&Hkeys.GetHKey(itype));
			SetAccels(false); 
			Hkeys.SaveHkeys();
		}
		return;
	}
	if(id==OpenSubs){
		//nie ma potrzeby pytać, bo w open file zapyta
		//if(SavePrompt(2)){return;}

		wxFileDialog *FileDialog1 = new wxFileDialog(this, _("Wybierz plik napisów"), 
			(GetTab()->VideoPath!="")? GetTab()->VideoPath.BeforeLast('\\') : 
			(subsrec.size()>0)?subsrec[subsrec.size()-1].BeforeLast('\\') : "", 
			"", _("Pliki napisów (*.ass),(*.ssa),(*.srt),(*.sub),(*.txt)|*.ass;*.ssa;*.srt;*.sub;*.txt|Pliki wideo z wbudowanymi napisami (*.mkv)|*.mkv"), wxFD_OPEN);
		if (FileDialog1->ShowModal() == wxID_OK){
			wxString file=FileDialog1->GetPath();
			if(file.AfterLast('.')=="mkv"){
				event.SetString(file);
				GetTab()->Grid1->OnMkvSubs(event);
			}
			else{
				OpenFile(file);
			}
		}
		FileDialog1->Destroy();
	}else if(id==OpenVideo){
		wxFileDialog* FileDialog2 = new wxFileDialog(this, _("Wybierz plik wideo"), 
			(videorec.size()>0)? videorec[videorec.size()-1].BeforeLast('\\') : "", 
			"", _("Pliki wideo(*.avi),(*.mkv),(*.mp4),(*.ogm),(*.wmv),(*.asf),(*.rmvb),(*.rm),(*.3gp),(*.mpg),(*.mpeg),(*.avs)|*.avi;*.mkv;*.mp4;*.ogm;*.wmv;*.asf;*.rmvb;*.rm;*.mpg;*.mpeg;*.3gp;*.avs|Wszystkie pliki (*.*)|*.*"), wxFD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, "wxFileDialog");
		if (FileDialog2->ShowModal() == wxID_OK){
			OpenFile(FileDialog2->GetPath());
		}
		FileDialog2->Destroy();
	}else if(id==Settings){
		OptionsDialog od(this,this);
		od.OptionsTree->ChangeSelection(0);
		od.ShowModal();
	}else if(id==SelectFromVideo){
		GetTab()->Grid1->SelVideoLine();
	}else if(id==Editor){
		HideEditor();
	}else if(id==PlayActualLine){
		TabPanel *tab = GetTab();
		tab->Edit->TextEdit->SetFocus();
		tab->Video->PlayLine(tab->Edit->line->Start.mstime, tab->Video->GetPlayEndTime(tab->Edit->line->End.mstime)/* - tab->Video->avtpf*/);
		//wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED,ID_BPLINE);
		//GetTab()->Video->OnAccelerator(evt);
	}else if(id==Quit){
		Close();
	}else if(id==About){
		KaiMessageBox(wxString::Format(_("Edytor napisów by Bjakja aka Bakura (bjakja7@gmail.com),\nwersja %s z dnia %s"),
			Options.progname.AfterFirst('v'), Options.GetReleaseDate()) + " \n\n"+
			_("Ten program to jakby moje zaplecze do nauki C++, więc mogą zdarzyć się różne błędy.\n\n")+
			_("Kainote zawiera w sobie części następujących projeków:\n")+
			L"wxWidgets - Copyright © Julian Smart, Robert Roebling et al;\n"+
			_("Color picker, wymuxowywanie napsów z mkv, audiobox, audio player, automation\ni kilka innych pojedynczych funkcji wzięte z Aegisuba -\n")+
			L"Copyright © Rodrigo Braz Monteiro;\n"\
			L"Hunspell - Copyright © Kevin Hendricks;\n"\
			L"Matroska Parser - Copyright © Mike Matsnev;\n"\
			L"CSRI - Copyright © David Lamparter;\n"\
			L"Vsfilter - Copyright © Gabest;\n"\
			L"FFMPEGSource2 - Copyright © Fredrik Mellbin;\n"\
			L"ICU - Copyright © 1995-2016 International Business Machines Corporation and others\n"\
			L"Boost - Copyright © Joe Coder 2004 - 2006.",
			"O Kainote");
		//L"FreeType - Copyright ©  David Turner, Robert Wilhelm, and Werner Lemberg;\n"\
		//L"Interfejs Avisynth - Copyright © Ben Rudiak-Gould et al.\n"
	}else if(id==Helpers){
		wxString Testers=L"Zły los, Nyah2211, Wincenty271, Ksenoform, Deadsoul,\nVessin, Xandros, Areki, Waski_jestem.";
		wxString Credits=_("Pomoc graficzna: (przyciski, obrazki do pomocy itd.)\n")+
			_("- Archer (pierwsze przyciski do wideo).\n")+
			_("- Kostek00 (przyciski do audio i narzędzi wizualnych).\n")+
			_("- Xandros (nowe przyciski do wideo).\n")+
			_("- Devilkan (ikony do menu i paska narzędzi, obrazki do pomocy).\n")+
			_("- Areki, duplex (tłumaczenie anglojęzyczne).\n \n")+
			_("Testerzy: (mniej i bardziej wprawieni użytkownicy programu)\n")+
			_("- Funki27 (pierwszy tester mający spory wpływ na obecne działanie programu\n")+
			_("i najbardziej narzekający na wszystko).\n")+
			_("- Sacredus (chyba pierwszy tłumacz używający trybu tłumaczenia,\n nieoceniona pomoc przy testowaniu wydajności na słabym komputerze).\n")+
			_("- Kostek00 (prawdziwy wynajdywacz błędów, miał duży wpływ na rozwój spektrum audio \n")+
			_("i głównego pola tekstowego, stworzył ciemny i jasny motyw, a także część ikon).\n")+
			_("- Devilkan (crashhunter, ze względu na swój system i przyzwyczajenia wytropił już wiele crashy,\n")+
			_("pomógł w poprawie działania narzędzi do typesettingu, wymyślił wiele innych usprawnień).\n")+
			_("- MatiasMovie (wyłapał parę crashy i zaproponował różne usprawnienia, pomaga w debugowaniu crashy).\n")+
			_("- mas1904 (wyłapał trochę błędów, pomaga w debugowaniu crashy).\n \n")+
			_("Podziękowania także dla osób, które używają programu i zgłaszali błędy.\n");
		KaiMessageBox(Credits+Testers,_("Lista osób pomocnych przy tworzeniu programu"));

	}else if(id==Help||id==ANSI){
		WinStruct<SHELLEXECUTEINFO> sei;
		wxString url=(id==Help)? L"file://" + Options.pathfull + L"\\Help\\Kainote pomoc.html" : L"http://animesub.info/forum/viewtopic.php?id=258715";
		sei.lpFile = url.c_str();
		sei.lpVerb = wxT("open");
		sei.nShow = SW_RESTORE;
		sei.fMask = SEE_MASK_FLAG_NO_UI; // we give error message ourselves

		ShellExecuteEx(&sei);
	}

}


void kainoteFrame::OnConversion(char form)
{
	TabPanel *tab=GetTab();
	//tab->Edit->OnVideo = true;
	tab->Grid1->Convert(form);
	//if(tab->Video->GetState()!=None){tab->Video->OpenSubs(tab->Grid1->GetVisible()/*SaveText()*/);tab->Video->Render();}

	tab->CTime->Contents();
	UpdateToolbar();
	tab->Edit->HideControls();
}


void kainoteFrame::OnAssProps()
{
	int x=-1,y=-1;
	if(GetTab()->Video->GetState()!=None){GetTab()->Video->GetVideoSize(&x,&y);}
	ScriptInfo sci(this,x,y);
	Grid *ngrid=GetTab()->Grid1;
	sci.title->SetValue(ngrid->GetSInfo("Title"));
	sci.script->SetValue(ngrid->GetSInfo("Original Script"));
	sci.translation->SetValue(ngrid->GetSInfo("Original Translation"));
	sci.editing->SetValue(ngrid->GetSInfo("Original Editing"));
	sci.timing->SetValue(ngrid->GetSInfo("Original Timing"));
	sci.update->SetValue(ngrid->GetSInfo("Script Updated By"));
	wxString oldx=ngrid->GetSInfo("PlayResX");
	wxString oldy=ngrid->GetSInfo("PlayResY");
	int nx=wxAtoi(oldx);
	int ny=wxAtoi(oldy);
	if(nx<1 && ny<1){nx=384;ny=288;}
	else if(nx<1){nx=(float)ny*(4.0/3.0);if(ny==1024){nx=1280;}}
	else if(ny<1){ny=(float)nx*(3.0/4.0);if(nx==1280){ny=1024;}}
	sci.width->SetInt(nx);
	sci.height->SetInt(ny);
	wxString matrix = ngrid->GetSInfo("YCbCr Matrix");
	int result = sci.matrix->FindString(matrix);
	if(matrix.IsEmpty() || result < 0){
		sci.matrix->SetSelection(0);
		result = 0;
	}else{
		sci.matrix->SetSelection(result);
	}
	wxString wraps=ngrid->GetSInfo("WrapStyle");
	int ws = wxAtoi(wraps);
	sci.wrapstyle->SetSelection(ws);
	wxString colls=ngrid->GetSInfo("Collisions");
	if(colls=="Reverse"){sci.collision->SetSelection(1);}
	wxString bords=ngrid->GetSInfo("ScaledBorderAndShadow");
	if (bords=="no"){sci.scaleBorderAndShadow->SetValue(false);}

	if(sci.ShowModal()==wxID_OK)
	{
		int newx=sci.width->GetInt();
		int newy=sci.height->GetInt();
		if(newx<1 && newy<1){newx=1280;newy=720;}
		else if(newx<1){newx=(float)newy*(4.0/3.0);}
		else if(newy<1){newy=(float)newx*(3.0/4.0);if(newx==1280){newy=1024;}}

		bool save=(!sci.noScaling->GetValue()&&(newx!=oldx||newy!=oldy));

		if(sci.title->GetValue()!=""){ if(sci.title->IsModified()){ngrid->AddSInfo("Title",sci.title->GetValue());} }
		else{ngrid->AddSInfo("Title","Kainote Ass File");}
		if(sci.script->IsModified()){ngrid->AddSInfo("Original Script",sci.script->GetValue());}
		if(sci.translation->IsModified()){ngrid->AddSInfo("Original Translation",sci.translation->GetValue());}
		if(sci.editing->IsModified()){ngrid->AddSInfo("Original Editing",sci.editing->GetValue());}
		if(sci.timing->IsModified()){ngrid->AddSInfo("Original Timing",sci.timing->GetValue());}
		if(sci.update->IsModified()){ngrid->AddSInfo("Script Updated By",sci.update->GetValue());}

		if(sci.width->IsModified()){ngrid->AddSInfo("PlayResX",wxString::Format("%i",newx));}
		if(sci.height->IsModified()){ngrid->AddSInfo("PlayResY",wxString::Format("%i",newy));}
		int newMatrix = sci.matrix->GetSelection();
		if(newMatrix != result){
			wxString val = sci.matrix->GetString(sci.matrix->GetSelection());
			ngrid->AddSInfo("YCbCr Matrix",val);
			GetTab()->Video->SetColorSpace(val);
		}

		if(ws != sci.wrapstyle->GetSelection()){ngrid->AddSInfo("WrapStyle",wxString::Format("%i",sci.wrapstyle->GetSelection()));}
		wxString collis=(sci.collision->GetSelection()==0)?"Normal":"Reverse";
		if(colls!=collis){ngrid->AddSInfo("Collisions",collis);}
		wxString bordas=(sci.scaleBorderAndShadow->GetValue())?"yes":"no";
		if(bords !=bordas){ ngrid->AddSInfo("ScaledBorderAndShadow",bordas);}


		if(save){
			int ox=wxAtoi(oldx);
			int oy=wxAtoi(oldy);
			ngrid->ResizeSubs((float)newx/(float)ox,(float)newy/(float)oy, sci.stretchScale->GetValue());
		}
		ngrid->SetModified(save);
		SetSubsResolution();
	}
}

void kainoteFrame::Save(bool dial, int wtab, bool changeLabel)
{
	TabPanel* atab=(wtab<0)? GetTab() : Tabs->Page(wtab);
	if(atab->Grid1->origform!=atab->Grid1->form
		||(Options.GetBool(SubsAutonaming) && atab->SubsName.BeforeLast('.') != atab->VideoName.BeforeLast('.') && atab->VideoName!="")
		||atab->SubsPath=="" || dial)
	{
		wxString extens=_("Plik napisów ");

		if (atab->Grid1->form<SRT){extens+="(*.ass)|*.ass";}
		else if (atab->Grid1->form==SRT){extens+="(*.srt)|*.srt";}
		else{extens+="(*.txt, *.sub)|*.txt;*.sub";};

		wxString path=(atab->VideoPath!="" && Options.GetBool(SubsAutonaming))? atab->VideoPath : atab->SubsPath;
		wxString name=path.BeforeLast('.');
		path=path.BeforeLast('\\');

		wxWindow *_parent=(atab->Video->isFullscreen)? (wxWindow*)atab->Video->TD : this;
		wxFileDialog saveFileDialog(_parent, _("Zapisz plik napisów"), 
			path, name ,extens, wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

		if (saveFileDialog.ShowModal() == wxID_OK){

			atab->SubsPath = saveFileDialog.GetPath();
			wxString ext=(atab->Grid1->form<SRT)? "ass" : (atab->Grid1->form==SRT)? "srt" : "txt";
			if(!atab->SubsPath.EndsWith(ext)){atab->SubsPath<<"."<<ext;}
			atab->SubsName=atab->SubsPath.AfterLast('\\');
			SetRecent();
		}else{return;}
	}
	atab->Grid1->SaveFile(atab->SubsPath);
	atab->Grid1->Modified=false;
	atab->Grid1->origform=atab->Grid1->form;
	if(changeLabel){Label(0,false,wtab);}
#if _DEBUG
	wxBell();
#endif
}

bool kainoteFrame::OpenFile(wxString filename,bool fulls)
{
	wxString ext=filename.Right(3).Lower();
	if(ext=="exe"||ext=="zip"||ext=="rar"||ext=="7z"){return false;}
	if(ext=="lua" || ext == "moon"){if(!Auto){Auto=new Auto::Automation(false);}Auto->Add(filename);return true;}
	TabPanel *tab=GetTab();

	bool found=false;
	bool nonewtab = true;
	wxString fntmp="";
	bool issubs=(ext=="ass"||ext=="txt"||ext=="sub"||ext=="srt"||ext=="ssa");

	if(tab->edytor && !(issubs && tab->VideoPath.BeforeLast('.')==filename.BeforeLast('.'))
		&&!(!issubs && tab->SubsPath.BeforeLast('.')==filename.BeforeLast('.'))){
			fntmp= FindFile(filename,issubs,!(fulls || tab->Video->isFullscreen) );
			if(fntmp!=""){found=true;if(!issubs){ext=fntmp.AfterLast('.');}}
	}

	if(Options.GetBool(OpenSubsInNewCard) && tab->SubsPath!="" &&
		!tab->Video->isFullscreen && issubs){
			//tab->Thaw();
			Tabs->AddPage(true);tab=Tabs->Page(Tabs->Size()-1);
			//tab->Freeze();
			nonewtab = false;
	}
	tab->Freeze();
	if(issubs||found){  
		wxString fname=(found&&!issubs)?fntmp:filename;
		if(nonewtab){if(SavePrompt(2)){tab->Thaw();return true;}}
		OpenWrite ow; 
		wxString s;
		if(!ow.FileOpen(fname, &s)){tab->Thaw();return false;}
		tab->Grid1->Loadfile(s,ext);

		if(ext=="ssa"){ext="ass";fname=fname.BeforeLast('.')+".ass";}
		tab->SubsPath=fname;
		tab->SubsName=fname.AfterLast('\\');


		if(ext=="ass"){
			wxString katal=tab->Grid1->GetSInfo("Last Style Storage");

			if(katal!=""){
				for(size_t i=0;i<Options.dirs.size();i++)
				{
					if(katal==Options.dirs[i]){
						Options.LoadStyles(katal);
						if(StyleStore::HasStore()){
							StyleStore *ss = StyleStore::Get();
							ss->Store->SetSelection(0,true);
							int chc=ss->catalogList->FindString(Options.acdir);
							ss->catalogList->SetSelection(chc);
						}
					}
				}
			}
		}

		if(tab->Video->GetState()!=None && !found){
			tab->Edit->OnVideo = true;
			bool isgood=tab->Video->OpenSubs((tab->edytor)? tab->Grid1->GetVisible()/*SaveText()*/ : 0);
			if(!isgood){KaiMessageBox(_("Otwieranie napisów nie powiodło się"), "Uwaga");}
		}
		SetRecent();


		Label();
		SetSubsResolution(!Options.GetBool(DontAskForBadResolution));
		if(!tab->edytor && !fulls && !tab->Video->isFullscreen){HideEditor();}
		if(!found){
			if(tab->Video->VFF && tab->Video->vstate != None && tab->Grid1->form == ASS){
				tab->Video->SetColorSpace(tab->Grid1->GetSInfo("YCbCr Matrix"));
			}
			tab->CTime->Contents();
			UpdateToolbar(); 
			tab->Thaw();
			return true;
		}
	}

	wxString fnname=(found && issubs)?fntmp:filename;

	bool isload=tab->Video->Load(fnname,tab->Grid1->SaveText(),fulls);


	tab->Thaw();
	tab->CTime->Contents();
	UpdateToolbar();
	Options.SaveOptions(true, false);
	if(!isload){return isload;}
	tab->Video->seekfiles=true;
	tab->Edit->Frames->Enable(!tab->Video->IsDshow);
	tab->Edit->Times->Enable(!tab->Video->IsDshow);

	//tab->Grid1->SetFocus();



	Tabs->GetTab()->Video->DeleteAudioCache();
	return true;  
}

void kainoteFrame::SetSubsResolution(bool showDialog)
{
	TabPanel *cur = GetTab();
	if(cur->Grid1->form != ASS){return;}
	wxString resolution = cur->Grid1->GetSInfo("PlayResX") +" x "+ cur->Grid1->GetSInfo("PlayResY");
	SetStatusText(resolution, 7);
	wxSize vsize;
	
	if(cur->Video->GetState()!=None && cur->edytor){
		vsize = cur->Video->GetVideoSize();
		wxString vres;
		vres<<vsize.x<<" x "<<vsize.y;
		if(vres!=resolution){
			wxColour warning = Options.GetColour(WindowWarningElements);
			StatusBar->SetLabelTextColour(5, warning);
			StatusBar->SetLabelTextColour(7, warning);
			badResolution=true;
			if(showDialog){
				ShowBadResolutionDialog(vres, resolution);
			}
			return;
		}
	}
	if(badResolution){
		wxColour nullcol;
		StatusBar->SetLabelTextColour(5, nullcol);
		StatusBar->SetLabelTextColour(7, nullcol);
		badResolution=false;
	}

}

void kainoteFrame::SetVideoResolution(int w, int h, bool showDialog)
{
	TabPanel *cur = GetTab();
	wxString resolution;
	resolution<<w<<" x "<<h;
	SetStatusText(resolution, 5);
	wxString sres = cur->Grid1->GetSInfo("PlayResX") +" x "+ cur->Grid1->GetSInfo("PlayResY");
	if(resolution != sres && sres.Len()>3 && cur->edytor){
		wxColour warning = Options.GetColour(WindowWarningElements);
		StatusBar->SetLabelTextColour(5, warning);
		StatusBar->SetLabelTextColour(7, warning);
		badResolution=true;
		if(showDialog){
			ShowBadResolutionDialog(resolution, sres);
		}
	}else if(badResolution){
		wxColour nullcol;
		StatusBar->SetLabelTextColour(5, nullcol);
		StatusBar->SetLabelTextColour(7, nullcol);
		badResolution=false;
	}
}

void kainoteFrame::ShowBadResolutionDialog(const wxString &videoRes, const wxString &subsRes)
{
	wxString info= wxString::Format(_("Rozdzielczości wideo i napisów różnią się."
		L"\nMożesz zmienić je teraz lub skorzystać z opcji we właściwościach ASS.\n\n"
		L"Rozdzielczość wideo: %s\nRozdzielczość napisów: %s\n\n"
		L"Dopasować rozdzielczość do wideo?\n\n"
		L"1. Dopasuj skrypt napisów do rozdzielczości wideo.\n"
		L"2. Zmień wyłącznie rozdzielczość napisów.\n"
		L"3. Pozostaw bez zmian.\n4. Wyłącz ostrzeżenie o niezgodności na stałe."),videoRes, subsRes);

	KaiMessageDialog dlg(this, info, _("Niezgodna rozdzielczość"), wxOK|wxYES_NO|wxHELP);
	dlg.SetHelpLabel(_("Wyłącz ostrzeżenie"));
	dlg.SetOkLabel(_("Dopasuj"));
	dlg.SetYesLabel(_("Zmień"));
	dlg.SetNoLabel(_("Zamknij"));
	int result = dlg.ShowModal();
	wxString vx = videoRes.BeforeFirst(' ');
	wxString sx = subsRes.BeforeFirst(' ');
	wxString vy = videoRes.AfterLast(' ');
	wxString sy = subsRes.AfterLast(' ');
	Grid *grid = GetTab()->Grid1;
	if(result == wxOK || result == wxYES){
		grid->AddSInfo("PlayResX",vx);
		grid->AddSInfo("PlayResY",vy);
		if(result  == wxOK){
			int ox=wxAtoi(sx);
			int oy=wxAtoi(sy);
			int newx=wxAtoi(vx);
			int newy=wxAtoi(vy);
			grid->ResizeSubs((float)newx/(float)ox,(float)newy/(float)oy,false);
		}
		grid->SetModified();
		SetSubsResolution();
	}else if(result == wxHELP){
		Options.SetBool(DontAskForBadResolution,true);
		Options.SaveOptions(true,false);
	}
}

//0 - subs, 1 - vids, 2 - auds
void kainoteFrame::SetRecent(short what)
{
	int idd=30000+(20*what);
	Menu *wmenu=(what==0)?SubsRecMenu : (what==1)? VidsRecMenu : AudsRecMenu;
	int size= (what==0)?subsrec.size() : (what==1)? videorec.size() : audsrec.size();
	wxArrayString recs=(what==0)?subsrec : (what==1)? videorec : audsrec;
	wxString path=(what==0)?GetTab()->SubsPath : (what==1)? GetTab()->VideoPath : GetTab()->Edit->ABox->audioName;

	for(int i=0;i<size;i++){
		if(recs[i]==path){
			recs.erase(recs.begin()+i);
			break;
		}
	}
	recs.Add(path);
	if(recs.size()>20){recs.erase(recs.begin());}
	if(what==0){subsrec=recs; Options.SetTable(SubsRecent,recs);}
	else if(what==1){videorec=recs; Options.SetTable(VideoRecent,recs);}
	else{audsrec=recs; Options.SetTable(AudioRecent,recs);}
}

//0 - subs, 1 - vids, 2 - auds
void kainoteFrame::AppendRecent(short what,Menu *_Menu)
{
	Menu *wmenu;
	if(_Menu){
		wmenu=_Menu;
	}else{
		wmenu=(what==0)?SubsRecMenu : (what==1)? VidsRecMenu : AudsRecMenu;
	}
	int idd=30000+(20*what);
	int size= (what==0)?subsrec.size() : (what==1)? videorec.size() : audsrec.size();

	wxArrayString recs=(what==0)?subsrec : (what==1)? videorec : audsrec;
	//wxLogStatus("count %i", wmenu->GetMenuItemCount());
	for(int j=wmenu->GetMenuItemCount()-1; j>=0; j--){
		//wxLogStatus("deleted %i", j);
		wmenu->Destroy(wmenu->FindItemByPosition(j));
	}

	for(int i=0;i<size;i++)
	{
		if(!wxFileExists(recs[i])){continue;}
		MenuItem* MI= new MenuItem(idd+i, recs[i].AfterLast('\\'), _("Otwórz ")+recs[i]);
		wmenu->Append(MI);
	}

	if(!wmenu->GetMenuItemCount()){
		MenuItem* MI= new MenuItem(idd, _("Brak"));
		MI->Enable(false);
		wmenu->Append(MI);
	}
}

void kainoteFrame::OnRecent(wxCommandEvent& event)
{
	int id=event.GetId();
	int Modif=event.GetInt();
	MenuItem* MI=0;
	if(id<30020){
		MI=SubsRecMenu->FindItem(id);
	}else if(id<30040){
		MI=VidsRecMenu->FindItem(id);
	}else{
		MI=AudsRecMenu->FindItem(id);
	}
	if(!MI){wxLogStatus("Item Menu Przepadł");return;}
	wxString filename=MI->GetHelp().AfterFirst(' ');
	//wxLogStatus("onrecent %i %i" + MI->GetHelp() + MI->GetLabel(), MI->GetId(), Modif);
	//return;
	//byte state[256];
	//if(GetKeyboardState(state)==FALSE){wxLogStatus(_("Nie można pobrać stanu klawiszy"));}
	if(Modif==wxMOD_CONTROL){
		wxWCharBuffer buf=filename.BeforeLast('\\').c_str();
		/*wchar_t **cmdline = new wchar_t*[3];
		cmdline[0] = L"explorer";
		cmdline[1] = buf.data();
		cmdline[2] = 0;
		long res = wxExecute(cmdline);
		delete cmdline;*/
		WinStruct<SHELLEXECUTEINFO> sei;
		sei.lpFile = buf;
		sei.lpVerb = wxT("explore");
		sei.nShow = SW_RESTORE;
		sei.fMask = SEE_MASK_FLAG_NO_UI; // we give error message ourselves


		if(!ShellExecuteEx(&sei)){wxLogStatus(_("Nie można otworzyć folderu"));}
		return;
	}
	if(id<30040){
		OpenFile(filename);
	}else{
		event.SetString(filename);
		OnOpenAudio(event);
	}
}


void kainoteFrame::OnSize(wxSizeEvent& event)
{
	wxSize size = GetSize();
	int fborder = 7;//(IsMaximized())? 0 : 5;
	int ftopBorder = 26;
	int menuHeight = Menubar->GetSize().GetHeight();
	int toolbarWidth = Toolbar->GetSize().GetWidth();
	int statusbarHeight = StatusBar->GetSize().GetHeight();
	Menubar->SetSize(fborder,ftopBorder,size.x-(fborder*2), menuHeight);
	Toolbar->SetSize(fborder,ftopBorder+menuHeight,toolbarWidth, size.y-menuHeight-statusbarHeight-ftopBorder-fborder);
	Tabs->SetSize(fborder+toolbarWidth,ftopBorder+menuHeight,size.x-toolbarWidth-(fborder*2), size.y-menuHeight-statusbarHeight-ftopBorder-fborder);
	StatusBar->SetSize(fborder,size.y - statusbarHeight - fborder,size.x-(fborder*2), statusbarHeight);
	
	event.Skip();
}


wxString kainoteFrame::FindFile(wxString fn,bool video,bool prompt)
{
	wxString filespec;
	wxString path=fn.BeforeLast('\\',&filespec);
	wxArrayString pliki;

	wxString plik="";

	wxDir kat(path);
	if(kat.IsOpened()){
		kat.GetAllFiles(path,&pliki,filespec.BeforeLast('.')+".*", wxDIR_FILES);
	} 
	if(pliki.size()<2){return "";}

	for(int i=0;i<(int)pliki.size();i++){
		wxString ext=pliki[i].AfterLast('.');
		if((!video&&(ext!="ass"&&ext!="txt"&&ext!="sub"&&ext!="srt"&&ext!="ssa"))
			||(video&&(ext!="avi"&&ext!="mp4"&&ext!="mkv"&&ext!="ogm"&&ext!="wmv"&&ext!="asf"&&ext!="rmvb"
			&&ext!="rm"&&ext!="3gp"&&ext!="ts"&&ext!="m2ts"&&ext!="m4v"&&ext!="flv"))  //&&ext!="avs" przynajmniej do momentu dorobienia obsługi przez avisynth
			){ }else{plik=pliki[i];break;}
	}
	if(plik!=""&&prompt){
		if (KaiMessageBox(wxString::Format(_("Wczytać %s o nazwie %s?"), 
			(video)? _("wideo") :_("napisy"), plik.AfterLast('\\')),
			_("Potwierdzenie"), wxICON_QUESTION | wxYES_NO, this) == wxNO ){plik="";} 
	}
	pliki.clear();
	return plik;
}


void kainoteFrame::OnP5Sec(wxCommandEvent& event)
{
	GetTab()->Video->Seek(GetTab()->Video->Tell()+5000);
}

void kainoteFrame::OnM5Sec(wxCommandEvent& event)
{
	GetTab()->Video->Seek(GetTab()->Video->Tell()-5000);

}

TabPanel* kainoteFrame::GetTab()
{
	return Tabs->GetPage();
}

void kainoteFrame::Label(int iter,bool video, int wtab)
{
	wxString whiter;
	if(iter>0){whiter<<iter<<"*";}
	TabPanel* atab=(wtab<0)? GetTab() : Tabs->Page(wtab);

	/*MEMORYSTATUSEX statex;
	statex.dwLength = sizeof (statex);
	GlobalMemoryStatusEx (&statex);
	int div=1024;
	int availmem=statex.ullAvailVirtual/div;
	int totalmem=statex.ullTotalVirtual/div;
	wxString memtxt= wxString::Format(" RAM: %i KB / %i KB", totalmem-availmem, totalmem);*/
	wxString name=(video)?atab->VideoName : atab->SubsName;
	SetLabel(whiter+name+" - "+Options.progname /*+ memtxt*/);
	if(name.Len()>35){name=name.SubString(0,35)+"...";}
	Tabs->SetPageText((wtab<0)? Tabs->GetSelection() : wtab, whiter+name);
}

void kainoteFrame::SetAccels(bool _all)
{
	std::vector<wxAcceleratorEntry> entries;
	entries.resize(2);
	entries[0].Set(wxACCEL_CTRL, (int) 'T', ID_ADDPAGE);
	entries[1].Set(wxACCEL_CTRL, (int) 'W', ID_CLOSEPAGE);

	for(auto cur=Hkeys.hkeys.begin(); cur!=Hkeys.hkeys.end(); cur++){
		if(cur->second.Accel=="" || cur->first.Type!='G'){continue;}
		int id=cur->first.id;
		if(id>=6850){
			if(id>=30100){
				Bind(wxEVT_COMMAND_MENU_SELECTED, &kainoteFrame::OnRunScript, this, id);
			}
			entries.push_back(Hkeys.GetHKey(cur->first, &cur->second));
		}else if(id>6000){
			MenuItem *item=Menubar->FindItem(id);
			if(!item){wxLogStatus("no id %i", id); continue;}
			cur->second.Name=item->GetLabelText();
			wxAcceleratorEntry accel = Hkeys.GetHKey(cur->first, &cur->second);
			item->SetAccel(&accel);
			entries.push_back(accel);
		}//else if(id<6001){break;}
		if(!entries[entries.size()-1].IsOk()){
			entries.pop_back();
		}
	}
	//Menubar->SetAccelerators();
	wxAcceleratorTable accel(entries.size(), &entries[0]);
	Tabs->SetAcceleratorTable(accel);

	if(!_all){return;}
	for(size_t i=0;i<Tabs->Size();i++)
	{
		Tabs->Page(i)->SetAccels();
	}
}


void kainoteFrame::InsertTab(bool sel)
{
	Tabs->AddPage(sel);
}

bool comp(wxString first, wxString second)
{
	return (first.CmpNoCase(second)<0);
}

void kainoteFrame::OpenFiles(wxArrayString files,bool intab, bool nofreeze, bool newtab)
{
	std::sort(files.begin(),files.end(),comp);
	wxArrayString subs;
	wxArrayString videos;
	for(size_t i=0;i<files.size();i++){
		wxString ext=files[i].AfterLast('.').Lower();
		if(ext=="ass"||ext=="ssa"||ext=="txt"||ext=="srt"||ext=="sub"){
			subs.Add(files[i]);
		}else if(ext=="lua" || ext=="moon"){
			if(!Auto){Auto=new Auto::Automation(false);}
			Auto->Add(files[i]);
		}
		else if(ext!="exe" && ext!="zip" && ext!="rar" && ext!="7z"){
			videos.Add(files[i]);
		}

	}

	if(files.size()==1){
		OpenFile(files[0],(videos.size()==1&&Options.GetBool(VideoFullskreenOnStart)));
		videos.Clear();subs.Clear();files.Clear();
		return;
	}
	bool askForRes = !Options.GetBool(DontAskForBadResolution);
	Freeze();
	GetTab()->Hide();
	size_t maxx=(subs.size()>videos.size())?subs.size() : videos.size();

	for(size_t i=0;i<maxx;i++)
	{

		if((i>=Tabs->Size() || Tabs->Page(Tabs->iter)->SubsPath!="" ||
			Tabs->Page(Tabs->iter)->VideoPath!="") && !intab){
				InsertTab(false);
		}
		TabPanel *tab=GetTab();
		if(i<subs.size()){
			wxString ext=subs[i].AfterLast('.').Lower();
			OpenWrite ow;
			wxString s;
			if(!ow.FileOpen(subs[i], &s)){
				break;
			}else{
				tab->Grid1->Loadfile(s,ext);
			}

			if(ext=="ssa"){ext="ass";subs[i]=subs[i].BeforeLast('.')+".ass";}
			tab->SubsPath=subs[i];
			tab->SubsName=tab->SubsPath.AfterLast('\\');
			if(!tab->edytor){HideEditor();}
			SetRecent();

			Label();
			SetSubsResolution(askForRes);
		}
		if(i<videos.size()){
			bool isload=tab->Video->Load(videos[i],(tab->edytor)? tab->Grid1->SaveText() : 0);

			if(!isload){
				if(tab->Video->IsDshow){KaiMessageBox(_("Plik nie jest poprawnym plikiem wideo albo jest uszkodzony,\nbądź brakuje kodeków czy też splittera"), _("Uwaga"));}
				break;
			}
			tab->Edit->Frames->Enable(!tab->Video->IsDshow);
			tab->Edit->Times->Enable(!tab->Video->IsDshow);

			tab->Video->seekfiles=true;

		}
		tab->CTime->Contents();

	}

	Thaw();
	//taka kolejność naprawia błąd znikających zakładek
	int w,h;
	Tabs->GetClientSize(&w,&h);
	Tabs->RefreshRect(wxRect(0,h-25,w,25),false);
	GetTab()->Show();
	UpdateToolbar();

	files.Clear();
	subs.Clear();
	videos.Clear();
	Tabs->GetTab()->Video->DeleteAudioCache();
	Options.SaveOptions(true, false);
}

void kainoteFrame::OnPageChange(wxCommandEvent& event)
{
	if(Tabs->Size()<2){return;}
	int step=(event.GetId()==NextTab)? Tabs->iter+1 : Tabs->iter-1;
	if(step<0){step=Tabs->Size()-1;}
	else if(step>=(int)Tabs->Size()){step=0;}
	Tabs->ChangePage(step);
}


void kainoteFrame::OnPageChanged(wxCommandEvent& event)
{
	wxString whiter;
	TabPanel *cur=Tabs->GetPage();
	int iter=cur->Grid1->file->Iter();
	if(iter>0 && cur->Grid1->Modified){whiter<<iter<<"*";}
	wxString name=(!cur->edytor)? cur->VideoName : cur->SubsName;
	SetLabel(whiter+name+" - "+Options.progname);
	if(cur->Video->GetState()!=None){
		SetStatusText(getfloat(cur->Video->fps)+" FPS",4);
		wxString tar;
		tar<<cur->Video->ax<<" : "<<cur->Video->ay;
		SetStatusText(tar,6);
		int x, y;
		cur->Video->GetVideoSize(&x, &y);
		tar.Empty();
		tar<<x<<" x "<<y;
		SetStatusText(tar,5);
		cur->Video->RefreshTime();

		STime kkk1;
		kkk1.mstime=cur->Video->GetDuration();
		SetStatusText(kkk1.raw(SRT),3);
		if(cur->edytor){
			SetStatusText(cur->VideoName,8);
		}
		else{SetStatusText("",8);}
		cur->Video->SetScaleAndZoom();
	}else{
		SetStatusText("",8);
		SetStatusText("",6);
		SetStatusText("",5);
		SetStatusText("",4);
		SetStatusText("",3);
		SetStatusText("",2);
		SetStatusText("",1);
	}
	if(cur->Grid1->form == ASS){
		SetSubsResolution();
	}else{
		SetStatusText("",7);
	}
	if(cur->edytor){cur->Grid1->SetFocus();}else{cur->Video->SetFocus();}
	cur->Grid1->UpdateUR(false);

	UpdateToolbar();

	cur->Grid1->SetFocus();
	if(Tabs->iter!=Tabs->GetOldSelection() && Options.GetBool(MoveTimesLoadSetTabOptions)){
		cur->CTime->RefVals(Tabs->Page( Tabs->GetOldSelection() )->CTime);
		//if(Options.GetBool("Grid save without enter")){
			//Tabs->Page( Tabs->GetOldSelection() )->Edit->Send(false);
		//}
	}

	if(Options.GetBool(AutoSelectLinesFromLastTab)){
		Grid *old=Tabs->Page(Tabs->GetOldSelection())->Grid1;
		if(old->FirstSel()>-1){
			cur->Grid1->SelVideoLine(old->GetDial(old->FirstSel())->Start.mstime);
		}
	}
	if(StyleStore::HasStore() && StyleStore::Get()->IsShown()){StyleStore::Get()->LoadAssStyles();}
	if(FR){FR->Reset();FR->ReloadStyle();}
}

void kainoteFrame::HideEditor()
{
	TabPanel *cur=GetTab();

	cur->edytor = !cur->edytor;

	cur->Grid1->Show(cur->edytor);

	cur->Edit->Show(cur->edytor);

	if(cur->edytor){//Załączanie Edytora

		cur->BoxSizer1->Detach(cur->Video);
		cur->BoxSizer2->Prepend(cur->Video, 0, wxEXPAND|wxALIGN_TOP, 0);
		cur->BoxSizer1->InsertSpacer(1,3);
		cur->Video->panelHeight = 66;
		cur->Video->vToolbar->Show();
		if(cur->Video->GetState()!=None&&!cur->Video->isFullscreen){
			int sx,sy,vw,vh;
			Options.GetCoords(VideoWindowSize,&vw,&vh);
			if(vh<350){vh=350,vw=500;}
			cur->Video->CalcSize(&sx,&sy,vw,vh);
			cur->Video->SetMinSize(wxSize(sx,sy + cur->Video->panelHeight));
		}else{cur->Video->Hide();}
		if(Options.GetBool(MoveTimesOn)){
			cur->CTime->Show();
		}
		cur->BoxSizer1->Layout();
		Label();
		if(cur->Video->GetState()!=None){cur->Video->ChangeVobsub();}

	}
	else{//Wyłączanie edytora
		cur->Video->panelHeight = 44;
		cur->CTime->Hide();

		cur->BoxSizer1->Remove(1);

		if(!cur->Video->IsShown()){cur->Video->Show();}

		cur->BoxSizer2->Detach(cur->Video);

		cur->BoxSizer1->Add(cur->Video, 1, wxEXPAND|wxALIGN_TOP, 0);
		
		cur->Video->vToolbar->Hide();
		if(cur->Video->GetState()!=None && !cur->Video->isFullscreen && !IsMaximized()){
			int sx,sy, sizex, sizey;
			GetClientSize(&sizex, &sizey);
			sizex -= iconsize;
			sizey -= (cur->Video->panelHeight+ Tabs->GetHeight() + Menubar->GetSize().y + StatusBar->GetSize().y);

			cur->Video->CalcSize(&sx,&sy, sizex, sizey, false, true);

			SetClientSize(sx+iconsize,sy + cur->Video->panelHeight+ Tabs->GetHeight() + Menubar->GetSize().y + StatusBar->GetSize().y);

		}
		cur->Video->SetFocus();

		cur->BoxSizer1->Layout();

		if(cur->VideoName!=""){Label(0,true);}
		if(cur->Video->GetState()!=None){cur->Video->ChangeVobsub(true);}
		//cur->Video->vToolbar->Enable(false);
	}
	UpdateToolbar();
}

void kainoteFrame::OnPageAdd(wxCommandEvent& event)
{
	InsertTab();
}

void kainoteFrame::OnPageClose(wxCommandEvent& event)
{
	Tabs->DeletePage(Tabs->GetSelection());
	OnPageChanged(event);
}


void kainoteFrame::SaveAll()
{
	for(size_t i=0;i<Tabs->Size();i++)
	{
		if(!Tabs->Page(i)->Grid1->Modified){continue;}
		Save(false,i);
		Label(0,false,i);
	}

}

//return anulowanie operacji
bool kainoteFrame::SavePrompt(char mode, int wtab)
{
	TabPanel* atab=(wtab<0)? GetTab() : Tabs->Page(wtab);	
	if(atab->Grid1->file->Iter()>0 && atab->Grid1->Modified){
		wxWindow *_parent=(atab->Video->isFullscreen)? (wxWindow*)atab->Video->TD : this;
		int answer = KaiMessageBox(wxString::Format(_("Zapisać napisy o nazwie \"%s\" przed %s?"), 
			atab->SubsName, (mode==0)? _("zamknięciem programu") :
			(mode==1)? _("zamknięciem zakładki") : 
			(mode==2)? _("wczytaniem nowych napisów") : 
			_("usunięciem napisów")), 
			_("Potwierdzenie"), wxYES_NO | wxCANCEL, _parent);
		if(answer==wxCANCEL){return true;}
		if(answer==wxYES){
			Save(false, wtab);
		}
	}
	return false;
}


void kainoteFrame::UpdateToolbar()
{//disejblowanie rzeczy niepotrzebnych przy wyłączonym edytorze
	MenuEvent evt;
	OnMenuOpened(evt);
	Toolbar->Updatetoolbar();
}

void kainoteFrame::OnOpenAudio(wxCommandEvent& event)
{
	OpenAudioInTab(GetTab(), event.GetId(), event.GetString());
}

void kainoteFrame::OpenAudioInTab(TabPanel *tab, int id, const wxString &path)
{

	if(id==CloseAudio && tab->Edit->ABox){
		tab->Video->player=NULL; 
		tab->Edit->ABox->Destroy(); 
		tab->Edit->ABox=NULL; 
		tab->Edit->Layout();}
	else{

		if(!Hkeys.AudioKeys && !Hkeys.LoadHkeys(true)){KaiMessageBox(_("Nie można wczytać skrótów klawiszowych audio"), _("Błąd"));return;}
		if(!Options.AudioOpts && !Options.LoadAudioOpts()){KaiMessageBox(_("Nie można wczytać opcji audio"), _("Błąd"));return;}

		wxString Path;
		if(id==OpenAudio){
			wxFileDialog *FileDialog1 = new wxFileDialog(this, _("Wybierz plik audio"), 
				(tab->VideoPath!="")? tab->VideoPath.BeforeLast('\\') : (videorec.size()>0)?subsrec[videorec.size()-1].BeforeLast('\\') : "", 
				"", _("Pliki audio i wideo (*.wav),(*.w64),(*.aac),(*.mp3),(*.mp4),(*.mkv),(*.avi)|*.wav;*.w64;*.aac;*.mp3;*.mp4;*.mkv;*.avi|Wszystkie Pliki |*.*"), wxFD_OPEN);
			int result = FileDialog1->ShowModal();
			if (result == wxID_OK){
				Path=FileDialog1->GetPath();
			}
			FileDialog1->Destroy();
			if(result == wxID_CANCEL){return;}
		}
		if(id>30039){Path=path;}
		if(Path.IsEmpty()){Path=tab->VideoPath;}
		if(Path.IsEmpty()){return;}


		if(tab->Edit->ABox){
			tab->Edit->ABox->SetFile(Path,(id==40000));
			if(!tab->Edit->ABox->audioDisplay->loaded){
				tab->Edit->ABox->Destroy(); 
				tab->Edit->ABox=NULL;
			}else{SetRecent(2);}
		}
		else{
			tab->Edit->ABox=new AudioBox(tab->Edit, tab->Grid1);
			tab->Edit->ABox->SetFile(Path, (id==40000));

			if(tab->Edit->ABox->audioDisplay->loaded){
				tab->Edit->BoxSizer1->Prepend(tab->Edit->ABox, 0, wxLEFT | wxRIGHT | wxEXPAND, 4);
				//int sizew,sizeh;
				//Options.GetCoords("Video Window Size",&sizew,&sizeh);
				if (!tab->Video->IsShown()){
					tab->Edit->SetMinSize(wxSize(500,350));
				}
				tab->Layout();
				Tabs->Refresh(false);
				tab->Edit->ABox->audioDisplay->SetFocus();
				SetRecent(2);
			}else{tab->Edit->ABox->Destroy(); tab->Edit->ABox=NULL;}
		}
	}
}



//uses before menu is shown
void kainoteFrame::OnMenuOpened(MenuEvent& event)
{
	Menu *curMenu = event.GetMenu();
	//wxLogStatus("opened " + curMenu->GetTitle());

	if(curMenu==FileMenu)
	{
		AppendRecent();
	}
	else if(curMenu==VidMenu)
	{
		AppendRecent(1);
	}
	else if(curMenu==AudMenu)
	{
		AppendRecent(2);
	}
	else if(curMenu==AutoMenu)
	{
		if(!Auto){Auto=new Auto::Automation();}
		Auto->BuildMenu(&AutoMenu);
		SetAccels();
	}
	TabPanel *tab = GetTab();
	bool enable=(tab->Video->GetState()!=None);
	bool editor=tab->edytor;
	for(int i = PlayPauseG; i<=SetVideoAtEnd; i++)
	{
		Menubar->Enable(i,(i<SetStartTime)? enable : enable && editor);
	}
	enable = (tab->Video->VFF!=NULL);
	Menubar->Enable(GoToPrewKeyframe, enable);
	Menubar->Enable(GoToNextKeyframe, enable);
	//kolejno numery id
	char form = tab->Grid1->form;
	bool tlmode = tab->Grid1->transl;
	for(int i=SaveSubs;i<=ViewSubs;i++){//po kolejne idy zajrzyj do enuma z pliku h, ostatni jest Automation
		enable=true;

		if(i>=ASSProperties&&i<ConvertToASS){enable=form<SRT;}//menager stylów i sinfo
		else if(i==ConvertToASS){enable=form>ASS;}//konwersja na ass
		else if(i==ConvertToSRT){enable=form!=SRT;}//konwersja na srt
		else if(i==ConvertToMDVD){enable=form!=MDVD;}//konwersja na mdvd
		else if(i==ConvertToMPL2){enable=form!=MPL2;}//konwersja na mpl2
		else if(i==ConvertToTMP){enable=form!=TMP;}//konwersja na tmp
		if((i>=ConvertToASS && i<=ConvertToTMP) && tlmode){enable=false;}
		if(i==ViewAudio || i==CloseAudio){enable= tab->Edit->ABox!=0;}
		if((i==ViewVideo || i==ViewAll )||i==AudioFromVideo){
			enable= tab->Video->GetState()!=None;
			if(i!=AudioFromVideo){enable = (enable && !tab->Video->isOnAnotherMonitor);}
		}
		if(i==SaveTranslation){enable=tlmode;}
		Menubar->Enable(i, editor && enable);
	}
	//specjalna poprawka do zapisywania w trybie tłumaczenia, jeśli jest tlmode, to zawsze ma działać.
	tab->Edit->TlMode->Enable((editor && form==ASS && tab->SubsPath!=""));
	//if(curMenu){Menubar->ShowMenu();}
}

//void kainoteFrame::OnMenuClick(wxCommandEvent &event)
//{
//	wxLogStatus("menu click %i", event.GetId());
//	auto action =  Auto->Actions.find(event.GetId());
//	if(action!=Auto->Actions.end()){
//		action->second.Run();
//	}
//}



void kainoteFrame::OnChangeLine(wxCommandEvent& event)
{

	int idd=event.GetId();
	if(idd<JoinWithPrevious){//zmiana linijki
		GetTab()->Grid1->NextLine((idd==PreviousLine)?-1 : 1);}
	else{//scalanie dwóch linijek
		GetTab()->Grid1->OnJoin(event);
	}
}

void kainoteFrame::OnDelete(wxCommandEvent& event)
{
	int idd=event.GetId();
	if(idd==Remove){
		GetTab()->Grid1->DeleteRows();
	}
	else{
		GetTab()->Grid1->DeleteText();
	}

}

void kainoteFrame::OnClose1(wxCloseEvent& event)
{
	size_t curit=Tabs->iter;
	for(size_t i=0;i<Tabs->Size();i++)
	{
		Tabs->iter=i;
		if(SavePrompt(0)){Tabs->iter=curit;return;}
	}
	Tabs->iter=curit;
	event.Skip();
}

void kainoteFrame::OnAudioSnap(wxCommandEvent& event)
{
	TabPanel *tab=GetTab();
	if(!tab->Edit->ABox){return;}
	int id=event.GetId();
	int time= (id==SnapWithStart)? tab->Edit->line->Start.mstime : tab->Edit->line->End.mstime;
	int time2= (id==SnapWithStart)? tab->Edit->line->End.mstime : tab->Edit->line->Start.mstime;
	int snaptime= tab->Edit->ABox->audioDisplay->GetBoundarySnap(time,1000,!Options.GetBool(AudioSnapToKeyframes),(id==SnapWithStart),true);
	//wxLogStatus(" times %i %i", snaptime, time);
	if(time!= snaptime){
		if(id==SnapWithStart){
			if (snaptime>=time2){return;}
			tab->Edit->StartEdit->SetTime(STime(snaptime), false, 1);
			tab->Edit->StartEdit->SetModified(true);
		}
		else{
			if (snaptime<=time2){return;}
			tab->Edit->EndEdit->SetTime(STime(snaptime), false, 2);
			tab->Edit->EndEdit->SetModified(true);
		}
		STime durTime = tab->Edit->EndEdit->GetTime() - tab->Edit->StartEdit->GetTime();
		if(durTime.mstime<0){durTime.mstime=0;}
		tab->Edit->DurEdit->SetTime(durTime, false, 1);
		tab->Edit->Send(false);
		tab->Edit->ABox->audioDisplay->SetDialogue(tab->Edit->line,tab->Edit->ebrow);
		tab->Video->RefreshTime();
	}
}


void kainoteFrame::OnOutofMemory()
{
	TabPanel *tab = Notebook::GetTab();

	if(tab->Grid1->file->maxx()>3){
		tab->Grid1->file->RemoveFirst(2);
		wxLogStatus(_("Zabrakło pamięci RAM, usunięto część historii"));
		return;
	}else if(Notebook::GetTabs()->Size()>1){
		for(size_t i=0; i<Notebook::GetTabs()->Size(); i++)
		{
			if( i!= Notebook::GetTabs()->GetSelection()){
				if(Notebook::GetTabs()->Page(i)->Grid1->file->maxx()>3){
					Notebook::GetTabs()->Page(i)->Grid1->file->RemoveFirst(2);
					wxLogStatus(_("Zabrakło pamięci RAM, usunięto część historii"));
					return;
				}
			}
		}
	}

	std::exit(1);
}

void kainoteFrame::OnRunScript(wxCommandEvent& event)
{
	if(!Auto){Auto=new Auto::Automation(true, true);}
	else if(Auto->Scripts.size()<1){Auto->ReloadScripts(true);}
	wxString name = Hkeys.GetName(idAndType(event.GetId()));
	if(!name.StartsWith("Script ")){ KaiMessageBox(wxString::Format(_("Skrót o nazwie '%s' nie należy do skrypru.")),_("Błąd"));return;}
	else{name = name.Mid(7);}
	wxString path = name.BeforeLast('-');
	int wmacro = 0;
	if(path.IsEmpty()){path = name;}
	else{wmacro = wxAtoi(name.AfterLast('-'));}
	Auto::LuaScript *script = Auto->FindScript(path);
	if(!script){
		Auto->Add(path);
		script = Auto->ASSScripts[Auto->ASSScripts.size()-1];
	}else{
		if(script->CheckLastModified(true)){script->Reload();}
	}
	auto macro = script->GetMacro(wmacro);
	if(macro){
		TabPanel *pan = GetTab();
		if(macro->Validate(pan)){
			macro->Run(pan);
		}else{
			KaiMessageBox(wxString::Format(_("Warunki skryptu Lua '%s' nie zostały spełnione"), script->GetPrettyFilename()),_("Błąd"));
		}
	}else{
		KaiMessageBox(wxString::Format(_("Błąd wczytywania skryptu Lua: %s\n%s"), script->GetPrettyFilename(), script->GetDescription()),_("Błąd"));
		Auto->OnEdit(script->GetFilename());	
	}
}

DEFINE_ENUM(Id,IDS)