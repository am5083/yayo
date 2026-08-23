#include "../src/board.hpp"
#include "../src/movegen.hpp"
#include "../src/eval.hpp"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
using namespace Yayo;
// flip colors of a fen: mirror ranks, swap case, swap side to move, swap castling
std::string flipFen(const std::string& fen){
    std::istringstream ss(fen); std::string pl, stm, cs, ep, hm, fm; ss>>pl>>stm>>cs>>ep>>hm>>fm;
    std::vector<std::string> ranks; std::string cur; for(char c: pl){ if(c=='/'){ranks.push_back(cur);cur="";} else cur+=c;} ranks.push_back(cur);
    std::reverse(ranks.begin(), ranks.end());
    std::string out; for(size_t i=0;i<ranks.size();i++){ for(char c: ranks[i]){ if(isalpha(c)) out+= isupper(c)?tolower(c):toupper(c); else out+=c;} if(i+1<ranks.size()) out+='/';}
    std::string ncs; for(char c: cs){ if(c=='-') ncs+=c; else ncs += isupper(c)?tolower(c):toupper(c);}
    std::string nep = ep; if(ep!="-"){ nep[1] = (ep[1]=='3')?'6':'3'; }
    return out+" "+(stm=="w"?"b":"w")+" "+ncs+" "+nep+" "+hm+" "+fm;
}
int main(int argc,char**argv){
    init_arrays(); initMvvLva();
    std::ifstream f(argv[1]); std::string line; int n=0, asym=0, traceMismatch=0;
    EvalWeights w; Score* weights=(Score*)&w;
    while(getline(f,line) && n<2000){
        std::string fen = line.substr(0, line.find(" c9")); // strip result if present
        if(fen.find("[")!=std::string::npos) fen = fen.substr(0, fen.find("[")-1);
        Board b; b.setFen(fen);
        Trace t; int e = Eval<TRACE>(b,t).eval();
        std::string ff = flipFen(fen); Board b2; b2.setFen(ff);
        int e2 = Eval<NO_TRACE>(b2).eval();
        if (e != e2) { if(asym<3) std::cout<<"ASYM "<<e<<" vs "<<e2<<" :: "<<fen<<"\n"; asym++; }
        // linear model check
        int *tr=(int*)&t; int ph = 4*popcount(b.pieces(QUEEN))+2*popcount(b.pieces(ROOK))+popcount(b.pieces(BISHOP))+popcount(b.pieces(KNIGHT)); int mg=std::min(24,ph), eg=24-mg;
        double lin=(b.turn==BLACK?-TEMPO:TEMPO); for(int i=0,wi=0;wi<487;i+=2,wi++){ int F=tr[i]-tr[i+1]; if(F) lin += F*(double)(mg*MgScore(weights[wi])+eg*EgScore(weights[wi]))/24.0; }
        int se = (b.turn==BLACK)? -e : e;
        if (std::abs(lin-se) > 3) { if(traceMismatch<3) std::cout<<"TRACE MISMATCH lin="<<lin<<" static="<<se<<" :: "<<fen<<"\n"; traceMismatch++; }
        n++;
    }
    std::cout<<"positions="<<n<<" asymmetric="<<asym<<" traceMismatch(>3cp)="<<traceMismatch<<"\n";
}
