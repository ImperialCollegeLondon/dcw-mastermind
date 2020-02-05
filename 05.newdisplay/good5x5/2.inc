%!PS-Adobe-3.0
%%Pages: 1
%%EndComments

<< /PageSize [595 842] >> setpagedevice

%include "pslib/textsize"
%include "pslib/odd"
%include "pslib/max"
%include "pslib/colours"

% Shhhhhhhhh! The secret code is YBYYR

/maxguesses 4  def
/numholes   5  def


/g1 (RPYRB) def
/b1 1 def
/w1 2 def


/g2 (RRRGP) def
/b2 0 def
/w2 1 def


/g3 (BPPBY) def
/b3 0 def
/w3 2 def




%include "mmsupport.i"


%%Page: 1 1
%%PageOrientation: Portrait

/Helvetica-Bold findfont 13 scalefont setfont

/texth fontheight def

maxguesses shownumbers

[ g1 g2 g3 (     ) ]    showguesses

[ b1 w1 b2 w2 b3 w3 ]    showscores

showpage
