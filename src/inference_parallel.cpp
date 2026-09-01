// Single inclusion point for the complete inference engine. Rename the
// tokenizer entry points while including the implementation so we can replace
// only the model-aware byte-level codec without duplicating the transformer.
#define encode encode_legacy
#define decode decode_legacy
#include "inference_v4.cpp"
#undef encode
#undef decode

namespace piai::inference {

static std::array<std::string,256> correct_byte_encoder() {
    std::array<std::string,256> out{};
    std::vector<int> bs;
    bs.reserve(256);
    for (int b=33;b<=126;++b) bs.push_back(b);
    for (int b=161;b<=172;++b) bs.push_back(b);
    for (int b=174;b<=255;++b) bs.push_back(b);
    std::array<bool,256> used{};
    for (int b:bs) used[b]=true;
    for (int b=0;b<256;++b) if (!used[b]) bs.push_back(b);
    // GPT-2 bytes_to_unicode: 188 printable bytes keep their code point;
    // all remaining bytes are assigned sequential code points from 256.
    for (size_t i=0;i<bs.size();++i) {
        const int b=bs[i];
        const uint32_t cp=(i < 188) ? static_cast<uint32_t>(b)
                                     : 256u + static_cast<uint32_t>(i-188);
        out[b]=u8cp(cp);
    }
    return out;
}

static std::string decode_byte_level_correct(const std::string& s) {
    static const auto rev=[] {
        std::unordered_map<uint32_t,uint8_t> r;
        r.reserve(256);
        const auto e=correct_byte_encoder();
        for (int b=0;b<256;++b) {
            auto v=cps(e[b]);
            if (v.size()==1) r[v[0]]=static_cast<uint8_t>(b);
        }
        return r;
    }();
    std::string out;
    out.reserve(s.size());
    for (uint32_t cp:cps(s)) {
        auto it=rev.find(cp);
        if (it!=rev.end()) out.push_back(static_cast<char>(it->second));
        else out+=u8cp(cp);
    }
    return out;
}

static std::string decode_token_correct(const std::string& token, bool gpt2) {
    if (token.empty()) return {};
    if (token.size()==6 && token[0]=='<' && token[1]=='0' && token[2]=='x' && token[5]=='>') {
        auto hex=[](char c)->int {
            if (c>='0'&&c<='9') return c-'0';
            if (c>='a'&&c<='f') return c-'a'+10;
            if (c>='A'&&c<='F') return c-'A'+10;
            return -1;
        };
        const int a=hex(token[3]), b=hex(token[4]);
        if (a>=0 && b>=0) return std::string(1,static_cast<char>((a<<4)|b));
    }
    if (gpt2) return decode_byte_level_correct(token);
    std::string out=token;
    for (size_t p=0;(p=out.find("▁",p))!=std::string::npos;) {
        out.replace(p,3," ");
        ++p;
    }
    return out;
}

static std::vector<std::string> bpe_correct(const std::string& s,const TokState& st) {
    static const auto enc=correct_byte_encoder();
    std::string encoded;
    encoded.reserve(s.size()*2);
    for (unsigned char b:s) encoded+=enc[b];
    auto v=symbols(encoded);
    if (v.empty()) return v;
    for (;;) {
        int best=std::numeric_limits<int>::max();
        size_t at=v.size();
        for (size_t i=0;i+1<v.size();++i) {
            auto it=st.rank.find(v[i]+" "+v[i+1]);
            if (it!=st.rank.end() && it->second<best) {
                best=it->second;
                at=i;
            }
        }
        if (at==v.size()) break;
        v[at]+=v[at+1];
        v.erase(v.begin()+at+1);
    }
    return v;
}

bool Vocabulary::encode(const std::string& s,std::vector<int32_t>& out) const {
    out.clear();
    auto itst=states.find(this);
    if (itst==states.end()) return encode_legacy(s,out);
    const TokState& st=itst->second;
    if (!st.gpt2) return encode_legacy(s,out);

    size_t p=0;
    while (p<s.size()) {
        size_t special_end=std::string::npos;
        int32_t special_id=-1;
        if (s[p]=='<') {
            size_t e=s.find("|>",p+2);
            if (e!=std::string::npos) {
                e+=2;
                std::string x=s.substr(p,e-p);
                auto id=st.id.find(x);
                if (id!=st.id.end() && st.special.find(x)!=st.special.end()) {
                    special_end=e;
                    special_id=id->second;
                }
            }
        }
        if (special_end!=std::string::npos) {
            out.push_back(special_id);
            p=special_end;
            continue;
        }
        size_t next=s.find("<|",p);
        std::string chunk=(next==std::string::npos)?s.substr(p):s.substr(p,next-p);
        if (chunk.empty()) { p=next; continue; }
        for (const auto& piece:smollm_split(chunk)) {
            if (piece.empty()) continue;
            for (const auto& sym:bpe_correct(piece,st)) {
                auto id=st.id.find(sym);
                if (id==st.id.end()) return false;
                out.push_back(id->second);
            }
        }
        if (next==std::string::npos) break;
        p=next;
    }
    return !out.empty();
}

std::string Vocabulary::decode(int32_t id) const {
    if (id<0 || static_cast<size_t>(id)>=tokens_.size()) return {};
    auto it=states.find(this);
    const bool gpt2=(it!=states.end() && it->second.gpt2);
    return decode_token_correct(tokens_[static_cast<size_t>(id)].text,gpt2);
}

} // namespace piai::inference
