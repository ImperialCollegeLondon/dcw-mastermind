%!PS-Adobe-3.0
%%Pages: 1
%%EndComments

<< /PageSize [595 842] >> setpagedevice

%include "pslib/textsize"
%include "pslib/odd"
%include "pslib/max"
%include "pslib/colours"

% Shhhhhhhhh! The secret code is RRPYP

/maxguesses 3  def
/numholes   5  def


/g1 (BYPGP) def
/b1 2 def
/w1 1 def


/g2 (RYRPP) def
/b2 2 def
/w2 3 def




%include "mmsupport.i"


%%Page: 1 1
%%PageOrientation: Portrait

/Helvetica-Bold findfont 13 scalefont setfont

/texth fontheight def

maxguesses shownumbers

[ g1 g2 (     ) ]    showguesses

[ b1 w1 b2 w2 ]    showscores

showpage
