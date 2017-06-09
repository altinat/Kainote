//  Copyright (c) 2017, Marcin Drob

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

#include "OptionsPanels.h"
//#include "Hotkeys.h"
#include "OptionsDialog.h"
#include "KaiStaticBoxSizer.h"
#include "KaiStaticText.h"
#include "KaiTextCtrl.h"
#include "KaiCheckBox.h"

SubtitlesProperties::SubtitlesProperties(wxWindow *parent, OptionsDialog *optionsDialog)
	: wxWindow(parent, -1)
{
	wxBoxSizer *main = new wxBoxSizer(wxVERTICAL);
	KaiStaticBoxSizer *StaticBox1 = new KaiStaticBoxSizer(wxVERTICAL,this, _("Informacje o napisach"));
	//wxFlexGridSizer *GridSizer=new wxFlexGridSizer(3,5,5);
	const int numFields = 6;
	wxString fieldNames[numFields] = {_("Tytu�"), _("Autor"), _("T�umaczenie"), _("Korekta"), _("Timing"), _("Edycja")};
	CONFIG fieldValues[numFields] = {ASSPropertiesTitle, ASSPropertiesScript, ASSPropertiesTranslation, 
		ASSPropertiesEditing, ASSPropertiesTiming, ASSPropertiesUpdate};
	CONFIG fieldOnValues[numFields] = {ASSPropertiesTitleOn, ASSPropertiesScriptOn, ASSPropertiesTranslationOn, 
		ASSPropertiesEditingOn, ASSPropertiesTimingOn, ASSPropertiesUpdateOn};

	for(int i = 0; i < numFields; i++){
		KaiTextCtrl *field = new KaiTextCtrl(this, -1,Options.GetString(fieldValues[i]),wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
		KaiCheckBox *fieldOn = new KaiCheckBox(this, -1, "", wxDefaultPosition, wxSize(18,-1));
		fieldOn -> SetValue(Options.GetBool(fieldOnValues[i]));
		wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(new KaiStaticText(this, -1, fieldNames[i]),2,wxEXPAND);
		sizer->Add(field,5,wxEXPAND);
		sizer->Add(fieldOn,0,wxEXPAND|wxLEFT,4);
		StaticBox1->Add(sizer,0,wxEXPAND|wxALL,5);
		optionsDialog->ConOpt(field, fieldValues[i]);
		optionsDialog->ConOpt(fieldOn, fieldOnValues[i]);
	}
	
	//StaticBox1->Add(GridSizer,0,wxEXPAND|wxALL,5);
	main->Add(StaticBox1,0,wxEXPAND,0);
	const int numCheckbox = 3;

	KaiCheckBox *option = new KaiCheckBox(this, -1, _("Zawsze pytaj o zmian� informacji w napisach"), wxDefaultPosition, wxSize(18,-1));
	option->SetValue(Options.GetBool(ASSPropertiesAskForChange));
	main->Add(option,0,wxEXPAND|wxALL,5);
	optionsDialog->ConOpt(option, ASSPropertiesAskForChange);

	SetSizerAndFit(main);
}