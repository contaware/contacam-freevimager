


var gBVType='firefox',
    gBVVersion=20,
    gOSType='windows nt 5';

/* This source code is Copyright (c) Vibrant Media 2001-2006 and forms part of the
patent-pending Vibrant Media product "IntelliTXT" (sm). */
function lES(url){
var e = document.createElement("script");
e.src = url;
e.type="text/javascript";
document.getElementsByTagName("body")[0].appendChild(e);
}

function iG(){
gHN=new Array();
gAN=new Array();
gSN=new Array('<b>','<h1>','<h2>','<h3>','<h4>','<h5>','<strong>','<em>','<i>','cpp-keyword','cpp-literal','cpp-comment','!ForumTable','smalltext');
}

function kwM(){
var ad=fABDID(aAD,gDI);
if(null==ad)return;
window.status=ad.t.replace(/\&pound\;/, '?');
}

var gWriteStyleBlock=1,
gCLM=1,
gK=new Array(),
gKL=new Array();
gK[0]='servers';
gK[1]='application';

gITXTNIdx=0;
function cpl(s){
var p=s.indexOf(' '),
c=1;
while(p>=0){
p=s.indexOf(' ',p+1);
c++;
}
return c;
}
for (i=0;i<gK.length;i++)
gKL[i]=cpl(gK[i]);
function pKWL(){
aAD.push(new iA(2194429,'servers','Server Solutions from Intel','Energy&#45;efficient Internet, email, & web servers for your business needs','','www%2eintel%2ecom','','',2,0,0,'','','8bcac6af-4364-4aed-bf14-7af638021610','4c5a3505ce469b28957197aa7c392be5',0,11366,2,0,0,0,0.126,0,'','','','',0,0,'',0, 60000, 0));
aAD.push(new iA(2683982,'application','Record-breaking Sun x64 Systems','Sun\'s world-class x64 servers deliver incredible performance, power efficiency and scalability benefits. FREE 60-day trial at sun.com.','','www%2esun%2ecom','','',2,0,0,'','','659e2f68-9f6c-45fc-ace4-d017768d1535','d13959d6a2ba65dcc0bb51f638f28fd0',0,120307,2,0,0,0,0.202,0,'','','','',0,0,'',0, 60000, 0));
 itxtprep();
 hIT(aAD,aKW,gSI,0);
}

function doIt(){

ipartid=1369;
ibid=2745;
ipid=5102;
ts='20061106074114';
mk=2;
server='codeproject.us.intellitxt.com';
cc='us';
rcc='ch';
itxtreg='--';
itxtdma='0';
ulS='font-weight:normal;font-size:100%;text-decoration:underline;border-bottom:darkgreen 0.075em solid;padding-bottom: 1px;color:darkgreen;background-color:transparent;padding-bottom:1px';
hS='font-weight:normal;font-size:100%;text-decoration:underline;border-bottom:darkgreen 0.2em solid;padding-bottom: 1px;color:darkgreen;background-color:transparent;padding-bottom:1px';
iEulS='';
iEhS='';
bWTL=true;
sWTU='http://secure.codeproject.com/info/intelliTXT.asp';
bTtl=false;
iCF=0;
gDBL=0;
kwpn=1;
gTI='';
gSI=1;
gSID='937626843c786bd725f88813b4712922';
mTL=50;
gDrag=1;
gOFlt=0;
gIRB=0;
gTTt=1500;
gDASB=0;

iTTC=0;

gLCL=4071;
if ('v1'==gCM)
mTL=0;

ndbST=new Date().getTime();
iDW();
if (gCM=='v1')
gITXTN=gIA2();
else
gITXTN=gIA(gdB, gCM, 0);
var bCF=0;

var tTXT='',aIEE;

if(tTXT.length==0)tTXT=document.title;
var aCs=new Array();
if (gITXTN!=null && gITXTN.length)
for(var i=0;i<gITXTN.length;i++){
var re2=/\\/g,
re3=/[\(\)]|<br>/g,
re4=/(\w+):\/\/([^\/:]+)(:\d*)?([^# ]*)/g,
re5=/<a[\s]+[^>]*?href[\s]?=[\s\"\']+(.*?)[\"\']+.*?>([^<]+|.*?)?<\/a>/g,
re7=/(\s{2,})/g,
re8=/(\s{2,})/g,
re9=/\n/g;
var re6=/[^-|^a-z|^A-Z|^0-9|^\x7f-\xff|^\/|^#|^+]/g,
sContents=tmA(gT(gITXTN[i]).replace(re2,'/').replace(re7,' ').replace(re3,' ').replace(re4,' ').replace(re5,' ').replace(re6,' ').replace(re8,'  '));

if (sContents.length>0)
{
aCs[aCs.length]=sContents+' zxz ';
gCL+=aCs[aCs.length-1].length;
}
}

if (aCs.length>0) gCL+=5;

if ((bCF==1 && gCL) || (gIRB==1 && bCF==0 && gCL>(gLCL+5)) || (gDTo==false && gIRB==1 && bCF==0 && Math.abs(gCL-gLCL)>=5)) {
gINt=' zxz ';
for(var i=0;i<aCs.length;i++)
gINt+=aCs[i];
var pTx=gINt,
nC=0,cn=0,sTx='',sSptTs=(new Date()).getTime(),chnkSz=(iNBc?250:500);
if (escape(pTx).length>chnkSz) {
   
sTx=pTx.substring(0,chnkSz);
while (sTx.length==chnkSz) {
    
lES('http://'+server+'/v3/chunks.jsp?ipid='+ipid+'&sid='+gSID+'&ts='+sSptTs+'&enc='+gENC+'&cn='+cn+'&tx='+escape(sTx));

nC+=chnkSz;cn++;
sTx=pTx.substring(nC, nC+chnkSz);
}
   
lES('http://'+server+'/v3/chunks.jsp?ipid='+ipid+'&sid='+gSID+'&nbc='+iNBc+'&ts='+sSptTs+'&enc='+gENC+'&cn='+cn+'&tx='+escape(sTx));
var sSpt = 'http://'+server+'/v3/spit.jsp?ipid='+ipid+'&ts='+sSptTs+'&cc='+cc+'&rcc='+rcc+'&reg='+itxtreg+'&dma='+itxtdma+'&cn='+cn+'&cf='+iCF+'&mk='+mk+'&kwpn='+kwpn+'&sid='+gSID+'&si='+gSI+'&rdtm='+iRDTM+'&rrm='+iRRM+'&rcpcm='+iRCPCM+'&ripsm='+iRIPSM+'&rorm='+iRORM+'&wpsm='+iWPSM+'&wpim='+iWPIM+'&dfc='+iDFC+'&rp='+bYCR+'&so='+iSo+'&nbc='+iNBc+'&adi='+iADi+'&ac=0&dat=6,11,12,17,18,22,25&irb='+gIRB+'&enc='+gENC+'&ims=2&iek=0&iel=5&ieak=0&iedm=0&iebc=cyan&ttxt='+escape(tTXT)+'&ru='+escape(sRU);

window.setTimeout('lES(\''+sSpt+'\');', 250);
} else

lES('http://'+server+'/v3/spit.jsp?ipid='+ipid+'&ts='+sSptTs+'&cc='+cc+'&rcc='+rcc+'&reg='+itxtreg+'&dma='+itxtdma+'&cf='+iCF+'&mk='+mk+'&kwpn='+kwpn+'&sid='+gSID+'&si='+gSI+'&rdtm='+iRDTM+'&rrm='+iRRM+'&rcpcm='+iRCPCM+'&ripsm='+iRIPSM+'&rorm='+iRORM+'&wpsm='+iWPSM+'&wpim='+iWPIM+'&dfc='+iDFC+'&rp='+bYCR+'&so='+iSo+'&nbc='+iNBc+'&adi='+iADi+'&ac=0&dat=6,11,12,17,18,22,25&irb='+gIRB+'&enc='+gENC+'&ims=2&iek=0&iel=5&ieak=0&iedm=0&iebc=cyan&ttxt='+escape(tTXT)+'&ru='+escape(sRU)+'&tx='+escape(pTx));

gSpitTimer = window.setInterval('checkSpit();', 5000);
} else if (gCL)

 {

if ((gITXTN == null) || (gITXTN != null && gITXTN.length==0))
gITXTN=gIA(gdB, gCM, 0);
if (gITXTN != null && gITXTN.length)
if (gCL>0) rKWL(250);

}
}
var ndbST,
iRDTM=100000000,
iRRM=100000,
iRCPCM=10000,
iRIPSM=1000,
iRORM=10000,
iWPSM=75,
iWPIM=50,
iDFC=0,
iTTl=1,
iTTp=0,
iSo=0,
iNBc=0,
iADi=0,
sKPs='~',
sKFs='|',
sRU='http://www.codeproject.com/useritems/googlemap.asp',
gMR=1000,
bWAC=0,
bYCR=0,
gMNC=255,
gTTD=500,
iIEK=0,
sIEFH='',
iDAH=0,
sTTBG='#EEEEEE',
sTTHV='#FFFFE0',
sTTTc='#0000DE',
sTTHTc='#0000DE',
sTTDc='#000000',
sTTAc='#008000',
sTTHAc='#008000',
gFES=0,
gSDCv=0,
gPmoe=0;
var sspl='Advertisement',
swti='what\'s this?',
sbn='Buy now',
scls='close',
sCC='$',
sEet='End time',
sEcb='Place bid',
sEsn='Seller',
ierl='Related Links',
iept='Page Title',
iert='Relevancy';
var gNeedProtos=1;

var prevOnload=(window.onload?window.onload:new Function()),
itxtInited=0,
itxtPrevOnloadCalled=0;
function itxtOnloader() {
 
startIntelliTXT(0);
}
function itxtStateChange() {
 
startIntelliTXT(1);
}
function startIntelliTXT(v) {
 
if (!v) {
if( itxtPrevOnloadCalled )
return;
itxtPrevOnloadCalled=1;
prevOnload();
}
if (itxtInited) return;
 
itxtInited=1;
lES('http://codeproject.us.intellitxt.com/v3/func_033.js?v=200611031223');
}
window.onload=itxtOnloader;
window.setTimeout('startIntelliTXT(1);',3000);

