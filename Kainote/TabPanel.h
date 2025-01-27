//  Copyright (c) 2016 - 2020, Marcin Drob

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


#pragma once

#include "KaiPanel.h"
//#include "ShiftTimes.h"
#include "VideoBox.h"
#include "KainoteFrame.h"
#include "TabPanel.h"
#include "KaiWindowResizer.h"
#include "SubsGrid.h"
#include "Editbox.h"

#include <wx/sizer.h>

class ShiftTimes;

class TabPanel : public KaiPanel
{
public:
	TabPanel(wxWindow *parent, KainoteFrame *kai, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize);
	virtual ~TabPanel();
	bool Hide();
	
	SubsGrid* grid;
	EditBox* edit;
	VideoBox* video;
	ShiftTimes* shiftTimes;

	wxBoxSizer* MainSizer;
	wxBoxSizer* VideoEditboxSizer;
	wxBoxSizer* GridShiftTimesSizer;

	void SetAccels(bool onlyGridAudio = false);
	void SetVideoWindowSizes(int w, int h, bool allTabs);
	bool SetFont(const wxFont &font);

	bool editor;
	bool audioHotkeysLoaded = false;


	wxString SubsName;
	wxString VideoName;
	wxString SubsPath;
	wxString VideoPath;
	wxString AudioPath;
	wxString KeyframesPath;
	int lastFocusedWindowId = 0;
	KaiWindowResizer* windowResizer;
private:

	bool holding;
	void OnFocus(wxChildFocusEvent& event);
	void OnSize(wxSizeEvent & evt);
	DECLARE_EVENT_TABLE()
};

