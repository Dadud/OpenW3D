#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;
struct Entry { std::uint32_t crc{}, offset{}, size{}; std::string name; };
struct Mix { std::uint32_t header_offset{}, names_offset{}; std::vector<Entry> entries; };
static std::uint32_t read_u32(std::ifstream& f) { unsigned char b[4]{}; if (!f.read(reinterpret_cast<char*>(b), 4)) throw std::runtime_error("unexpected end of file"); return std::uint32_t(b[0]) | (std::uint32_t(b[1])<<8) | (std::uint32_t(b[2])<<16) | (std::uint32_t(b[3])<<24); }
static void write_u32(std::ofstream& f, std::uint32_t v) { unsigned char b[4]{char(v),char(v>>8),char(v>>16),char(v>>24)}; f.write(reinterpret_cast<char*>(b),4); }
static void seek(std::ifstream& f, std::uint64_t p) { f.seekg(static_cast<std::streamoff>(p)); if (!f) throw std::runtime_error("invalid archive offset"); }
static std::string json_escape(const std::string& s) { std::string r; for (unsigned char c:s) { if(c=='\\')r+="\\\\"; else if(c=='"')r+="\\\""; else if(c=='\n')r+="\\n"; else if(c=='\r')r+="\\r"; else if(c=='\t')r+="\\t"; else if(c<0x20)r+='?'; else r+=char(c); } return r; }
static Mix read_mix(const fs::path& path) {
    std::ifstream f(path,std::ios::binary|std::ios::ate); if(!f) throw std::runtime_error("cannot open "+path.string()); auto file_size=std::uint64_t(f.tellg()); seek(f,0); char magic[4]{}; if(!f.read(magic,4)||std::string(magic,4)!="MIX1") throw std::runtime_error("not a MIX1 archive");
    Mix mix{read_u32(f),read_u32(f),{}}; if(mix.header_offset+4>file_size||mix.names_offset>=file_size) throw std::runtime_error("MIX offsets outside file"); seek(f,mix.header_offset); auto count=read_u32(f); if(count>1000000||mix.header_offset+4ull+count*12ull>file_size) throw std::runtime_error("invalid MIX entry table"); mix.entries.resize(count);
    for(auto&e:mix.entries){e.crc=read_u32(f);e.offset=read_u32(f);e.size=read_u32(f);if(std::uint64_t(e.offset)+e.size>file_size)throw std::runtime_error("MIX entry outside file");} seek(f,mix.names_offset); auto name_count=read_u32(f); if(name_count!=count)throw std::runtime_error("MIX name count mismatch");
    for(auto&e:mix.entries){unsigned char n{};if(!f.read(reinterpret_cast<char*>(&n),1)||n>255)throw std::runtime_error("invalid MIX filename");e.name.resize(n);if(!f.read(e.name.data(),n))throw std::runtime_error("truncated MIX names");while(!e.name.empty()&&e.name.back()=='\0')e.name.pop_back();} return mix;
}
static void write_json(const fs::path& path,const Mix&m,const fs::path&out){std::ofstream o(out);if(!o)throw std::runtime_error("cannot write index");o<<"{\n  \"schema_version\": 1,\n  \"format\": \"MIX1\",\n  \"archive\": \""<<json_escape(path.string())<<"\",\n  \"header_offset\": "<<m.header_offset<<",\n  \"names_offset\": "<<m.names_offset<<",\n  \"entry_count\": "<<m.entries.size()<<",\n  \"entries\": [\n";for(size_t i=0;i<m.entries.size();++i){auto&e=m.entries[i];o<<"    {\"name\": \""<<json_escape(e.name)<<"\", \"crc\": "<<e.crc<<", \"offset\": "<<e.offset<<", \"size\": "<<e.size<<"}"<<(i+1==m.entries.size()?"\n":",\n");}o<<"  ]\n}\n";}
static void write_package(const fs::path& archive,const Mix&m,const fs::path&out){
    std::ifstream in(archive,std::ios::binary);std::ofstream o(out,std::ios::binary);if(!in||!o)throw std::runtime_error("cannot open package input/output");
    o.write("OWPK",4);write_u32(o,1);write_u32(o,std::uint32_t(m.entries.size()));write_u32(o,16);std::uint64_t table_size=0;for(auto&e:m.entries)table_size+=12+e.name.size();write_u32(o,std::uint32_t(16+table_size));std::uint32_t payload=0;for(auto&e:m.entries){write_u32(o,std::uint32_t(e.name.size()));o.write(e.name.data(),e.name.size());write_u32(o,e.crc);write_u32(o,payload);write_u32(o,e.size);payload+=e.size;}
    std::vector<char> buf(1<<20);for(auto&e:m.entries){seek(in,e.offset);std::uint32_t left=e.size;while(left){auto n=std::min<std::uint32_t>(left,buf.size());if(!in.read(buf.data(),n))throw std::runtime_error("truncated MIX payload");o.write(buf.data(),n);left-=n;}}
}
static void extract(const fs::path&a,const Entry&e,const fs::path&o){std::ifstream f(a,std::ios::binary);std::ofstream out(o,std::ios::binary);if(!f||!out)throw std::runtime_error("cannot open extract path");seek(f,e.offset);std::vector<char>b(1<<20);std::uint32_t left=e.size;while(left){auto n=std::min<std::uint32_t>(left,b.size());if(!f.read(b.data(),n))throw std::runtime_error("truncated asset");out.write(b.data(),n);left-=n;}}
int main(int argc,char**argv){try{if(argc<2||std::string(argv[1])=="--help"){std::cout<<"Usage: openw3d-mix-index ARCHIVE.mix [--output INDEX.json] [--extract NAME FILE] [--package FILE]\n";return argc<2;};fs::path archive=argv[1],index=archive.filename().string()+".json",package;std::string name;fs::path extracted;for(int i=2;i<argc;++i){std::string a=argv[i];if(a=="--output"&&i+1<argc)index=argv[++i];else if(a=="--extract"&&i+2<argc){name=argv[++i];extracted=argv[++i];}else if(a=="--package"&&i+1<argc)package=argv[++i];else throw std::runtime_error("unknown/incomplete option: "+a);}auto m=read_mix(archive);write_json(archive,m,index);if(!package.empty())write_package(archive,m,package);if(!name.empty()){auto it=std::find_if(m.entries.begin(),m.entries.end(),[&](auto&e){return e.name==name;});if(it==m.entries.end())throw std::runtime_error("asset not found: "+name);extract(archive,*it,extracted);}std::cout<<"Indexed "<<archive.string()<<": "<<m.entries.size()<<" entries\n";if(!package.empty())std::cout<<"Cooked package: "<<package.string()<<"\n";return 0;}catch(const std::exception&e){std::cerr<<"error: "<<e.what()<<'\n';return 1;}}
