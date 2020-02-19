//  Copyright (c) 2016, Marcin Drob

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

#include <wx/wx.h>

class KaiStaticBox : public wxStaticBox
{
public:
	KaiStaticBox(wxWindow *parent, const wxString& label);
	virtual ~KaiStaticBox(){};
	wxSize CalcBorders();
	bool Enable(bool enable = true);
private:
	//void OnSize(wxSizeEvent& event);
	//void OnPaint(wxPaintEvent& event);
	void PaintForeground(wxDC& dc, const RECT& rc);
	wxString label;
	int heightText;
};

class KaiStaticBoxSizer : public wxBoxSizer
{
public:
	KaiStaticBoxSizer(int orient, wxWindow *parent, const wxString& _label);
	virtual ~KaiStaticBoxSizer();
	void ShowItems( bool show );
	bool Enable(bool enable);
private:
	void RecalcSizes();
	wxSize CalcMin();
	bool Detach( wxWindow *window );
	KaiStaticBox *box;
};
