﻿

#include "Visuals.h"
#include "TabPanel.h"
#include <wx/regex.h>
//#include <regex>


Visuals *Visuals::Get(int Visual, wxWindow *_parent)
{
	Visuals *visual;
	switch(Visual)
	{
	case CHANGEPOS:
		visual=new Position();
		break;
	case MOVE:
		visual=new Move();
		break;
	case MOVEALL:
		visual=new MoveAll();
		break;
	case SCALE:
		visual=new Scale();
		break;
	case ROTATEZ:
		visual=new RotationZ();
		break;
	case ROTATEXY:
		visual=new RotationXY();
		break;
	case CLIPRECT:
		visual=new ClipRect();
		break;
	case VECTORCLIP:
	case VECTORDRAW:
		visual=new DrawingAndClip();
		break;
	default:
		visual=new Position();
		break;
	}

	visual->tab = (TabPanel*)_parent->GetParent();
	visual->Visual= Visual;
	return visual;
}


float Visuals::wspw=0;
float Visuals::wsph=0;

Visuals::Visuals()
{
	from = lastmove = to = D3DXVECTOR2(0,0);
	line=0;
	font=0;
	device=0;
	start=end=oldtime=0;
	blockevents=false;
	dummytext=NULL;
}

Visuals::~Visuals()
{
	SAFE_DELETE(dummytext);
}

void Visuals::SetVisual(int _start,int _end)
{
	int nx=0, ny=0;
	tab->Grid1->GetASSRes(&nx, &ny);
	SubsSize=wxSize(nx,ny);
	start=_start;
	end=_end;

	wspw=((float)SubsSize.x/(float)VideoSize.x);
	wsph=((float)SubsSize.y/(float)VideoSize.y);
	tab->Video->VisEdit=true;

	SetCurVisual();
	if(Visual==VECTORCLIP){
		SetClip(GetVisual(),true); return;
	}
	tab->Video->Render(false);
}

void Visuals::SizeChanged(wxSize wsize, LPD3DXLINE _line, LPD3DXFONT _font, LPDIRECT3DDEVICE9 _device)
{
	line=_line;
	font=_font;
	device=_device;
	VideoSize=wsize;
	wspw=((float)SubsSize.x/(float)wsize.x);
	wsph=((float)SubsSize.y/(float)wsize.y);

	HRN(device->SetFVF( D3DFVF_XYZ|D3DFVF_DIFFUSE), "fvf failed");
}

//drawarrow od razu przesuwa punkt tak by linia kończyła się przed strzałką
//from i to są koordynatami linii
//diff można przesunąć strzałkę w tył od punktu "to"
void Visuals::DrawArrow(D3DXVECTOR2 from, D3DXVECTOR2 *to, int diff)
{
	D3DXVECTOR2 pdiff=from- (*to);
	float len= sqrt((pdiff.x * pdiff.x) + (pdiff.y*pdiff.y));
	D3DXVECTOR2 diffUnits = (len==0)? D3DXVECTOR2(0,0) : pdiff / len;
	// długość może przyjmnować wartości ujemne, dlatego dajemy + strzałka nie była odwrotnie
	D3DXVECTOR2 pend=(*to) + (diffUnits * (12+diff));
	D3DXVECTOR2 halfbase = D3DXVECTOR2(-diffUnits.y, diffUnits.x) * 5.f;

	VERTEX v4[7];
	D3DXVECTOR2 v3[3];
	v3[0]= pend - diffUnits * 12;
	v3[1]= pend + halfbase;
	v3[2]= pend - halfbase;

	CreateVERTEX(&v4[0],v3[0].x,v3[0].y,0xAA121150);
	CreateVERTEX(&v4[1],v3[1].x,v3[1].y,0xAA121150);
	CreateVERTEX(&v4[2],v3[2].x,v3[2].y,0xAA121150);
	CreateVERTEX(&v4[3],v3[0].x,v3[0].y,0xFFBB0000);
	CreateVERTEX(&v4[4],v3[1].x,v3[1].y,0xFFBB0000);
	CreateVERTEX(&v4[5],v3[2].x,v3[2].y,0xFFBB0000);
	CreateVERTEX(&v4[6],v3[0].x,v3[0].y,0xFFBB0000);

	HRN(device->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 1, v4, sizeof(VERTEX) ),"primitive failed");
	HRN(device->DrawPrimitiveUP( D3DPT_LINESTRIP, 3, &v4[3], sizeof(VERTEX) ),"primitive failed");
	*to = pend;
}

void Visuals::DrawCross(D3DXVECTOR2 position, D3DCOLOR color, bool useBegin)
{
	D3DXVECTOR2 cross[4];
	cross[0].x = position.x-15.0f;
	cross[0].y = position.y;
	cross[1].x = position.x+15.0f;
	cross[1].y = position.y;
	cross[2].x = position.x;
	cross[2].y = position.y-15.0f;
	cross[3].x = position.x;
	cross[3].y = position.y+15.0f;
	if(useBegin){line->Begin();}
	line->Draw(cross,2,color);
	line->Draw(&cross[2],2,color);
	if(useBegin){line->End();}

}

void Visuals::DrawRect(D3DXVECTOR2 pos, bool sel, float rcsize)
{
	//line->End();
	D3DCOLOR fill = (sel)? 0xAAFCE6B1 : 0xAA121150;
	VERTEX v9[9];
	CreateVERTEX(&v9[0], pos.x-rcsize, pos.y-rcsize, fill);
	CreateVERTEX(&v9[1], pos.x+rcsize, pos.y-rcsize, fill);
	CreateVERTEX(&v9[2], pos.x-rcsize, pos.y+rcsize, fill);
	CreateVERTEX(&v9[3], pos.x+rcsize, pos.y+rcsize, fill);
	CreateVERTEX(&v9[4], pos.x-rcsize, pos.y-rcsize, 0xFFBB0000);
	CreateVERTEX(&v9[5], pos.x+rcsize, pos.y-rcsize, 0xFFBB0000);
	CreateVERTEX(&v9[6], pos.x+rcsize, pos.y+rcsize, 0xFFBB0000);
	CreateVERTEX(&v9[7], pos.x-rcsize, pos.y+rcsize, 0xFFBB0000);
	CreateVERTEX(&v9[8], pos.x-rcsize, pos.y-rcsize, 0xFFBB0000);

	HRN(device->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, v9, sizeof(VERTEX) ),"primitive failed");
	HRN(device->DrawPrimitiveUP( D3DPT_LINESTRIP, 4, &v9[4], sizeof(VERTEX) ),"primitive failed");
	//line->Begin();
}



void Visuals::DrawCircle(D3DXVECTOR2 pos, bool sel, float crsize)
{
	//line->End();
	D3DCOLOR fill = (sel)? 0xAAFCE6B1 : 0xAA121150;
	VERTEX v5[41];
	float rad =0.01745329251994329576923690768489f;

	float xx = pos.x;
	float yy = pos.y;
	CreateVERTEX(&v5[0], xx, yy, fill);
	for(int j=0; j<20; j++)
	{
		float xx1= pos.x + (crsize * sin ( (j*20) * rad ));
		float yy1= pos.y + (crsize * cos ( (j*20) * rad ));
		CreateVERTEX(&v5[j+1], xx1, yy1, fill);
		CreateVERTEX(&v5[j+21], xx1, yy1, 0xFFBB0000);
		xx=xx1;
		yy=yy1;

	}

	HRN(device->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 18, v5, sizeof(VERTEX) ),"primitive failed");
	HRN(device->DrawPrimitiveUP( D3DPT_LINESTRIP, 18, &v5[21], sizeof(VERTEX) ),"primitive failed");
	//line->Begin();
}

void Visuals::DrawDashedLine(D3DXVECTOR2 *vector, size_t vectorSize, int dashLen)
{

	D3DXVECTOR2 actualPoint[2];
	for(size_t i = 0; i < vectorSize - 1; i++){
		size_t iPlus1 = (i < (vectorSize-1))? i+1 : 0;
		D3DXVECTOR2 pdiff= vector[i] - vector[iPlus1];
		float len= sqrt((pdiff.x * pdiff.x) + (pdiff.y * pdiff.y));
		if(len==0){return;}
		D3DXVECTOR2 diffUnits = pdiff / len;
		float singleMovement = 1/(len/(dashLen*2));
		actualPoint[0] = vector[i];
		actualPoint[1] = actualPoint[0];
		for(float j = 0; j <= 1; j += singleMovement){
			actualPoint[1] -= diffUnits * dashLen;
			if(j+singleMovement>=1){actualPoint[1] = vector[iPlus1];}
			line->Draw(actualPoint,2,0xFFBB0000);
			actualPoint[1] -= diffUnits * dashLen;
			actualPoint[0] -= (diffUnits * dashLen)*2;
		}
	}
}


void Visuals::Draw(int time)
{
	if(!(time>=start && time<=end)){blockevents = true; return;}else if(blockevents){blockevents=false;}
	wxMutexLocker lock(clipmutex);
	line->SetAntialias(TRUE);
	line->SetWidth(2.0);

	DrawVisual(time);

	line->SetAntialias(FALSE);
	oldtime=time;
}



D3DXVECTOR2 Visuals::CalcMovePos()
{
	D3DXVECTOR2 ppos;
	int time= tab->Video->Tell();
	if(tbl[6]<6){tbl[4]=start; tbl[5]=end;}
	float tmpt= time - tbl[4];
	float tmpt1=tbl[5] - tbl[4];
	float actime= tmpt/tmpt1;
	float distx, disty;
	if(time < tbl[4]){distx=tbl[0], disty= tbl[1];}
	else if(time > tbl[5]){distx=tbl[2], disty= tbl[3];}
	else {
		distx= tbl[0] -((tbl[0]-tbl[2])*actime); 
		disty = tbl[1] -((tbl[1]-tbl[3])*actime);
	}
	ppos.x=distx, ppos.y=disty;
	return ppos;
}

D3DXVECTOR2 Visuals::GetPos(Dialogue *Dial, bool *putinBracket, wxPoint *TextPos){
	//no i zadanie na jutro, napisać tę funkcję i najlepiej jeszcze getmove i getscale, 
	//ładnie to ogarnąć w samych visualach a z editboxa wszystko wywalić, łącznie z clipami.
	*putinBracket=false;
	D3DXVECTOR2 result;
	Styles *acstyl=tab->Grid1->GetStyle(0,Dial->Style);
	bool istxttl = (tab->Grid1->transl && Dial->TextTl!="");
	wxString txt = (istxttl)? Dial->TextTl : Dial->Text;
	bool foundpos=false;
	wxRegEx pos("\\\\(pos|move)\\(([^\\)]+)\\)",wxRE_ADVANCED);
	if(pos.Matches(txt)){
		wxString txtpos = pos.GetMatch(txt,2);
		double posx=0, posy=0;
		wxString rest, rest1;
		bool res1 = txtpos.BeforeFirst(',', &rest).ToCDouble(&posx);
		bool res2 = rest.BeforeFirst(',', &rest1).ToCDouble(&posy);
		size_t startMatch,lenMatch;
		if(pos.GetMatch(&startMatch, &lenMatch, 0)){
			TextPos->x=startMatch;
			TextPos->y=lenMatch;
		}
		result = D3DXVECTOR2(posx,posy);
		if(res1&&res2){return result;}
	}else{
		result.x= (tab->Edit->line->MarginL!=0)? tab->Edit->line->MarginL : wxAtoi(acstyl->MarginL);
		result.y= (tab->Edit->line->MarginV!=0)? tab->Edit->line->MarginV : wxAtoi(acstyl->MarginV);
	}

	if(txt!="" && txt[0]=='{'){
		TextPos->x= 1;
		TextPos->y= 0;
	}else{
		TextPos->x= 0;
		TextPos->y= 0;
		*putinBracket=true;
	}
	int tmpan;
	tmpan=wxAtoi(acstyl->Alignment);
	wxRegEx an("\\\\an([0-9]+)",wxRE_ADVANCED);
	if(an.Matches(txt)){
		tmpan=wxAtoi(an.GetMatch(txt,1));
	}
	//D3DXVECTOR2 dsize = Notebook::GetTab()->Video->Vclips->CalcWH();
	int x, y;
	tab->Grid1->GetASSRes(&x, &y);
	if(tmpan % 3==2){
		result.x = (x/2);
	}
	else if(tmpan % 3==0){
		result.x = (Dial->MarginR!=0)? Dial->MarginR : wxAtoi(acstyl->MarginR);
		result.x = x - result.x;
	}
	if(tmpan < 4){
		result.y = (Dial->MarginV!=0)? Dial->MarginV : wxAtoi(acstyl->MarginV);
		result.y =  y - result.y;
	}else if(tmpan < 7){
		result.y = (y/2);
	}

	return result;
}

//pobieranie pozycji i skali, trzeba tu zrobić rozróznienie na tagi działające na całą linię i tagi miejscowe.
//W przypadku rysowania wektorowego, należy podać scale, w reszcie przypadków mozna olać wszystko bądź jedną wartość.
D3DXVECTOR2 Visuals::GetPosnScale(D3DXVECTOR2 *scale, byte *AN, double *tbl)
{
	bool beforeCursor=!(Visual>=VECTORCLIP || Visual== MOVE || Visual== CHANGEPOS);
	bool draw=(Visual == VECTORCLIP || Visual == VECTORDRAW);
	D3DXVECTOR2 ppos(0.0f,0.0f);
	EditBox *edit = tab->Edit;
	Grid *grid = tab->Grid1;
	wxString txt = edit->TextEdit->GetValue();
	MTextEditor *Editor = edit->TextEdit;
	if(grid->transl && txt==""){ txt = edit->TextEditTl->GetValue(); Editor = edit->TextEditTl;}


	Styles *acstyl=grid->GetStyle(0,edit->line->Style);
	bool foundpos=false;
	wxRegEx pos("\\\\(pos|move)\\(([^\\)]+)\\)",wxRE_ADVANCED);
	if(pos.Matches(txt) && tbl){
		wxString type=pos.GetMatch(txt,1);
		wxString txtpos = pos.GetMatch(txt,2);
		wxStringTokenizer tkz(txtpos,",");
		int ipos=0; //tbl[4]=0; tbl[5]=0;
		while(tkz.HasMoreTokens()&& ipos<6){
			wxString token=tkz.GetNextToken();
			if(!token.ToDouble(&tbl[ipos])){tbl[ipos]=0;}
			//wxLogStatus("move %i, %f",ipos, tbl[ipos]);
			ipos++;
		}
		tbl[4]+=edit->line->Start.mstime; tbl[5]+=edit->line->Start.mstime;
		tbl[6]=ipos;
		if(ipos>1){ppos.x=tbl[0];ppos.y=tbl[1];foundpos=true;}

	}else{
		if(tbl){tbl[6]=0;}
		ppos.x= (edit->line->MarginL!=0)? edit->line->MarginL : wxAtoi(acstyl->MarginL);
		ppos.y= (edit->line->MarginV!=0)? edit->line->MarginV : wxAtoi(acstyl->MarginV);
	}

	if(tbl && tbl[6]<4){
		VideoCtrl *video = tab->Video;
		int framestart=(video->IsDshow)? (((float)edit->line->Start.mstime/1000.0)*video->fps)+1 : video->VFF->GetFramefromMS(edit->line->Start.mstime);
		int frameend=(video->IsDshow)? ((float)edit->line->End.mstime/1000.0)*video->fps : video->VFF->GetFramefromMS(edit->line->End.mstime)-1;
		int msstart=(video->IsDshow)? ((framestart*1000)/video->fps) : video->VFF->GetMSfromFrame(framestart);
		int msend=(video->IsDshow)? ((frameend*1000)/video->fps) : video->VFF->GetMSfromFrame(frameend);
		int diff=edit->line->End.mstime - edit->line->Start.mstime;
		//wxLogStatus("czasy %i %i %i %i %i", edit->line->Start.mstime, msstart, edit->line->End.mstime, msend, diff);
		tbl[4]=abs( msstart - edit->line->Start.mstime);
		tbl[5]=diff - abs(edit->line->End.mstime - msend);
		tbl[4]+=edit->line->Start.mstime; tbl[5]+=edit->line->Start.mstime;
	}
	if(!beforeCursor){Editor->SetSelection(0,0);}

	wxString sxfd, syfd;
	bool scx=edit->FindVal("fscx([.0-9-]+)", &sxfd);
	bool scy=edit->FindVal("fscy([.0-9-]+)", &syfd);
	double fscx=100.0, fscy=100.0;
	if(scx){
		sxfd.ToDouble(&fscx);
	}else{
		acstyl->ScaleX.ToDouble(&fscx);
	}
	if(scy){
		syfd.ToDouble(&fscy);
	}else{
		acstyl->ScaleY.ToDouble(&fscy);
	}
	if(scale){
		scale->x=fscx/100;
		scale->y=fscy/100;
	}
	if(draw){
		wxRegEx drawscale;
		if(Visual==VECTORCLIP){
			*scale = D3DXVECTOR2(1,1);
			drawscale.Compile("\\\\i?clip\\(([0-9]+),", wxRE_ADVANCED);
		}else{
			drawscale.Compile("\\\\p([0-9]+)", wxRE_ADVANCED);
		}
		int dscale=1;
		if(drawscale.Matches(txt)){
			dscale = wxAtoi(drawscale.GetMatch(txt,1));
		}
		dscale= pow(2.f,(dscale-1.f));
		scale->x /= dscale;
		scale->y /= dscale;
	}
	if(Visual!=VECTORCLIP){
		int tmpan;
		tmpan=wxAtoi(acstyl->Alignment);
		wxRegEx an("\\\\an([0-9]+)",wxRE_ADVANCED);
		if(an.Matches(txt)){
			tmpan=wxAtoi(an.GetMatch(txt,1));
		}
		if(AN){*AN = tmpan;}
		if(foundpos){return ppos;}
		//D3DXVECTOR2 dsize = Notebook::GetTab()->Video->Vclips->CalcWH();
		int x, y;
		grid->GetASSRes(&x, &y);
		if(tmpan % 3==2){
			ppos.x = (x/2);
		}
		else if(tmpan % 3==0){
			ppos.x = (edit->line->MarginR!=0)? edit->line->MarginR : wxAtoi(acstyl->MarginR);
			ppos.x = x - ppos.x;
		}
		if(tmpan < 4){
			ppos.y = (edit->line->MarginV!=0)? edit->line->MarginV : wxAtoi(acstyl->MarginV);
			ppos.y =  y - ppos.y;
		}else if(tmpan < 7){
			ppos.y = (y/2);
		}
	}


	return ppos;
}
//funkcja zwraca 1 gdy mamy przesunięcie o nawias, 0 w przeciwhnym przypadku
int ChangeText(wxString *txt, const wxString &what, bool notinbracket, const wxPoint &pos)
{
	if(!notinbracket){
		txt->insert(0,"{"+what+"}");
		return 1;
	}
	if(pos.x<pos.y){txt->erase(txt->begin() + pos.x, txt->begin() + pos.y+1);}
	txt->insert(pos.x,what);
	return 0;
}

void Visuals::SetClip(wxString clip,bool dummy, bool redraw)
{
	
	EditBox *edit = tab->Edit;
	Grid *grid = tab->Grid1;
	bool isOriginal=(grid->transl && edit->TextEdit->GetValue()=="");
	//Editor
	MTextEditor *Editor=(isOriginal)? edit->TextEditTl : edit->TextEdit;
	if(clip==""){
		//
		wxString tmp;
		wxString txt = Editor->GetValue();
		if(edit->FindVal("(i?clip.)[^\\\\}]*", &tmp, txt)){
			//wxLogStatus("find");
			ChangeText(&txt,"",edit->InBracket,edit->Placed);
			txt.Replace("{}", "");
			Editor->SetTextS(txt, false);
			Editor->modified = true;
			//wxLogStatus("find");
			edit->Send(false);
			return;
		}
		tab->Video->VisEdit=false;
		if(!tab->Video->OpenSubs(tab->Grid1->GetVisible())){wxLogStatus(_("Nie można otworzyć napisów"));}
		tab->Video->VisEdit=true;
		if(redraw){tab->Video->Render();}
		
		return;
	}
	if(dummy){
		bool vis=false;
		//wxLogStatus("dummytext %i", (int)dummytext);
		if(!dummytext){
			if(Visual==VECTORCLIP){
				//wxPoint pos;
				wxString tmp="clip(";
				wxString txt=Editor->GetValue();
				bool fv=edit->FindVal("(i?clip.)[^\\\\}]*", &tmp,txt);
				wxString tmp1=(tmp[0]=='c')? "iclip(" : "clip(";
				wxString tclip= "\\"+tmp+clip+")";
				edit->Placed.x += tmp.Len()+ 1 + ChangeText(&txt,tclip,edit->InBracket,edit->Placed);
				edit->Placed.y=edit->Placed.x+clip.Len();
				dummytext= grid->GetVisible(&vis, &textplaced);
				if(!vis){SAFE_DELETE(dummytext);return;}
				dummytext->replace(textplaced.x,textplaced.y,txt);
				textplaced.y=txt.Len();
				int nx=0, ny=0;
				grid->GetASSRes(&nx, &ny);
				Dialogue *visdl=edit->line->Copy();
				visdl->Text="";
				visdl->Text<<"{\\p1\\bord0\\shad0\\fscx100\\fscy100\\1c&H000000&\\1a&H77&\\pos(0,0)\\an7\\"<<tmp1<<clip<<")}m 0 0 l "<<
					nx<<" 0 "<<nx<<" "<<ny<<" 0 "<<ny;
				(*dummytext)<<visdl->GetRaw();
				dumplaced.x=edit->Placed.x + textplaced.x; dumplaced.y=edit->Placed.y + textplaced.x;
				delete visdl;
				Editor->SetTextS(txt,false);
				//wxLogStatus("dummytext "+ *dummytext);
			}else{
				wxString tmp="";
				bool isf;
				size_t cliplen = clip.Len();
				Editor->SetSelection(0,0);
				isf=edit->FindVal("p([0-9]+)", &tmp);
				wxString txt=Editor->GetValue();
				if(!isf){
					ChangeText(&txt, "\\p1", edit->InBracket, edit->Placed);
				}
				
				txt.Replace("}{","");
				dummytext=grid->GetVisible(&vis, &textplaced);
				if(!vis){SAFE_DELETE(dummytext);return;}
				edit->FindVal("p([0-9]+)", &tmp);
				wxString afterP1 = txt.Mid(edit->Placed.y);
				//wxLogStatus("afterp1 "+ afterP1);
				int Mpos = -1;
				if(isf){Mpos = afterP1.find("m ");}
				if(Mpos== -1){Mpos = afterP1.find("}")+1;}
				wxString startM = afterP1.Mid(Mpos);
				//wxLogStatus("startm "+ startM);
				int endClip = startM.find("{");
				if(endClip == -1 && isf){endClip=startM.Len();clip+="{\\p0}";}
				else if(endClip == -1){endClip=0;clip+="{\\p0}";}
				txt.replace(Mpos + edit->Placed.y, endClip, clip);
				dummytext->replace(textplaced.x,textplaced.y,txt);
				textplaced.y=txt.Len();
				dumplaced.x=edit->Placed.y + Mpos + textplaced.x; dumplaced.y= dumplaced.x + cliplen;
				Editor->SetTextS(txt,false);

			}
		}else{
			//if(dumplaced.x<dumplaced.y){dummytext->erase(dummytext->begin()+dumplaced.x, dummytext->begin()+dumplaced.y);}
			dummytext->replace(dumplaced.x,dumplaced.y-dumplaced.x,clip);
			//ChangeText(dummytext,clip,true,dumplaced);
			//wxLogStatus(*dummytext);
			int oldy=dumplaced.y;
			dumplaced.y=dumplaced.x+clip.Len();
			textplaced.y += (dumplaced.y - oldy);
			if(Visual==VECTORCLIP){
				int endclip=dummytext->Find(')',true);
				int startclip=dummytext->Find('(',true);
				dummytext->replace(startclip+1, endclip-(startclip+1), clip);
			}
			wxString txt = dummytext->Mid(textplaced.x,textplaced.y);
			Editor->SetTextS(txt,false);//,false,true
			Editor->Refresh();
		}
		
		tab->Video->VisEdit=false;
		//wxLogStatus(*dummytext);
		wxString *dtxt=new wxString(*dummytext);
		if(!tab->Video->OpenSubs(dtxt)){wxLogStatus(_("Nie można otworzyć napisów"));}
		tab->Video->VisEdit=true;
		//if(redraw){
			tab->Video->Render();
		//}

	}
	else{

		Editor->modified=true;
		edit->UpdateChars(Editor->GetValue());
		tab->Video->VisEdit=true;
		if(edit->splittedTags){edit->TextEditTl->modified=true;}
		edit->Send(false,false,true);
	}
}

//Wstawianie visuali do tekstu linijki
void Visuals::SetVisual(wxString visual,bool dummy, int type)
{
	//wstawianie wisuali ale najpierw muszę sobie dać ich rozróżnianie
	EditBox *edit = tab->Edit;
	Grid *grid = tab->Grid1;

	bool isOriginal=(grid->transl && edit->TextEdit->GetValue()=="");
	//Editor
	MTextEditor *Editor=(isOriginal)? edit->TextEditTl : edit->TextEdit;

	if(dummy){
		wxString txt=Editor->GetValue();

		if(Visual==MOVE||Visual==CHANGEPOS||Visual==CLIPRECT){Editor->SetSelection(0,0);}
		wxString tmp;
		wxString xytype= (type==0)? "x" : "y";
		wxString frxytype= (type==1)? "x" : "y";

		wxString tagpattern= (type==100)? "(org).+" : (Visual==MOVE||Visual==CHANGEPOS)? "(move|pos).+" : (Visual==SCALE)? "(fsc"+xytype+").+" : (Visual==ROTATEZ)? "(frz?)[0-9-]+" : (Visual==ROTATEXY)? "(fr"+frxytype+").+" : (Visual==CLIPRECT)? "(i?clip).+" : "(fa"+xytype+").+";
		edit->FindVal(tagpattern, &tmp);

		if(type==2 && Visual>0){
			if(edit->Placed.x < edit->Placed.y){txt.erase(txt.begin() + edit->Placed.x, txt.begin() + edit->Placed.y+1);}
			wxString tagpattern= (Visual==SCALE)? "(fscx).+" : (Visual==ROTATEZ)? "(frz?)[0-9-]+" : (Visual==ROTATEXY)? "(frx).+" : "(fax).+";
			edit->FindVal(tagpattern, &tmp, txt);
		}
		/*if(!edit->InBracket){
			txt.insert(edit->Placed.x,"{"+visual+"}");
		}
		else{
			if(edit->Placed.x<edit->Placed.y){txt.erase(txt.begin() + edit->Placed.x, txt.begin() + edit->Placed.y+1);}
			txt.insert(edit->Placed.x, visual);
			Editor->SetSelection(edit->Placed.x, edit->Placed.x, true);
		}*/
		ChangeText(&txt,visual,edit->InBracket,edit->Placed);
		if(!dummytext){
			bool vis=false;
			dummytext= grid->GetVisible(&vis, &dumplaced);
			if(!vis){SAFE_DELETE(dummytext); return;}
		}else{
			Editor->SetTextS(txt,false,false);
			Editor->Refresh(false);
		}

		dummytext->replace(dumplaced.x,dumplaced.y,txt);
		dumplaced.y=txt.Len();

		wxString *dtxt=new wxString(*dummytext);
		if(!tab->Video->OpenSubs(dtxt)){wxLogStatus(_("Nie można otworzyć napisów"));}
		tab->Video->VisEdit=true;
		tab->Video->Render();
	}else{
		//Editor->Refresh(false);
		Editor->modified=true;
		tab->Video->VisEdit=true;
		if(edit->splittedTags){edit->TextEditTl->modified=true;}
		edit->Send(false,false,true);

	}
}
