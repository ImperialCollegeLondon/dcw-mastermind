%!PS-Adobe-3.0
%%Pages: 1
%%EndComments

<< /PageSize [595 842] >> setpagedevice

%include "pslib/textsize"
%include "pslib/odd"
%include "pslib/max"
%include "pslib/colours"

% Shhhhhhhhh! The secret code is YGBG

/maxguesses 3  def
/numholes   4  def


/g1 (RRYR) def
/b1 0 def
/w1 1 def


/g2 (GBGY) def
/b2 0 def
/w2 4 def




%include "mmsupport.i"


%%Page: 1 1
%%PageOrientation: Portrait

/Helvetica-Bold findfont 13 scalefont setfont

/texth fontheight def

maxguesses shownumbers

[ g1 g2 (     ) ]    showguesses

[ b1 w1 b2 w2 ]    showscores

showpage
