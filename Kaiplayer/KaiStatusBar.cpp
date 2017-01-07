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

#include "KaiStatusBar.h"
#include "config.h"
#include <wx\dcmemory.h>
#include <wx\dcclient.h>
#include <wx\log.h>

KaiStatusBar::KaiStatusBar(wxWindow *parent, int id, int style)
	:wxWindow(parent, id, wxDefaultPosition, wxSize(-1, 26))
	,bmp(NULL)
{
	Bind(wxEVT_SIZE, &KaiStatusBar::OnSize, this);
	Bind(wxEVT_PAINT, &KaiStatusBar::OnPaint, this);
	/*Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent &evt){
		wxLogStatus("focus");
	});*/
	SetMinSize(wxSize(200,26));
}

void KaiStatusBar::SetFieldsCount(int num, int *fields)
{
	for(int i = 0; i < num; i++){
		sizes.Add(fields[i]);
	}
	labels.resize(num);
}

void KaiStatusBar::OnSize(wxSizeEvent& event)
{
	Refresh(false);
}

void KaiStatusBar::OnPaint(wxPaintEvent& event)
{
	int w=0;
	int h=0;
	GetClientSize (&w, &h);
	if(w==0||h==0){return;}
	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < w || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = NULL;
	}
	if(!bmp){bmp=new wxBitmap(w,h);}
	tdc.SelectObject(*bmp);
	tdc.SetFont(GetFont());
	bool enabled = IsThisEnabled();
	wxColour wbg = Options.GetColour("Window Background");
	wxColour wfg = Options.GetColour("Window Text");
	tdc.SetBrush(wxBrush(wbg));
	tdc.SetPen(wxPen(wfg));
	tdc.DrawRectangle(0,0,w,h);
	wxArrayInt widths;
	CalcWidths(&widths);
	int posX = 1;
	for(size_t i = 0; i<widths.size(); i++){
		if(widths[i]>0 && i>0){
			tdc.SetPen(wfg);
			tdc.DrawLine(posX-1, 1, posX-1, h-1);
		}
		if(labels[i]==""){posX += widths[i]; continue;}
		wxColour bg = (background.size() > i && background[i].IsOk())? background[i] : wbg;
		tdc.SetTextForeground((foreground.size() > i && foreground[i].IsOk())? foreground[i] : wfg);
		tdc.SetBrush(bg);
		tdc.SetPen(bg);
		tdc.DrawRectangle(posX,1,widths[i]-1,h-2);
		wxRect cur(posX+4, 1, widths[i] - 5, h-2);
		tdc.SetClippingRegion(cur);
		tdc.DrawLabel(labels[i],cur,wxALIGN_CENTER_VERTICAL);
		tdc.DestroyClippingRegion();
		posX += widths[i];

	}
	tdc.DrawBitmap(wxBITMAP_PNG("gripper"), w-18, h-18);
	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}

void KaiStatusBar::CalcWidths(wxArrayInt *widths)
{
	widths->resize(sizes.size());
	int w, h, wRest=0, perspective = 0;
	GetClientSize(&w, &h);
	wRest = w;
	for( size_t i = 0; i < sizes.size(); i++){
		if(sizes[i] < 0){perspective += sizes[i]; continue;}
		if(labels[i]=="" || sizes[i]){(*widths)[i] = sizes[i]; wRest -= sizes[i]; continue;}
		wxSize size = GetTextExtent(labels[i]);
		(*widths)[i] = size.x + 10; wRest -= size.x + 10;
	}
	for( size_t i = 0; i < sizes.size(); i++){
		if(sizes[i] >= 0){continue;}
		(*widths)[i] = ((float)sizes[i] / (float)perspective) * wRest;
	}
}

void KaiStatusBar::SetLabelText(size_t field, const wxString &label)
{
	if(field >= labels.size()){return;}
	labels[field] = label;
	Refresh(false);
}
	
wxString KaiStatusBar::GetStatusText(size_t field) const
{
	if(field >= labels.size()){return "";}
	return labels[field];
}
	
void KaiStatusBar::SetLabelTextColour(size_t field, const wxColour &textColour)
{
	if(field >= foreground.size()){foreground.resize(field+1);}
	foreground[field] = textColour;
	Refresh(false);
}
	
void KaiStatusBar::SetLabelBackgroundColour(size_t field, const wxColour &backgroundColour)
{
	if(field >= background.size()){background.resize(field+1);}
	background[field] = backgroundColour;
	Refresh(false);
}