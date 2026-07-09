#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

struct Candidate {
    fs::path root;
    int score = 0;
    std::vector<std::string> hints;
    std::uintmax_t mix_count = 0;
    std::uintmax_t w3d_count = 0;
    std::uintmax_t map_count = 0;
    std::uintmax_t data_files = 0;
    bool always_dat = false;
    bool localized = false;
    bool executable = false;
    bool launcher_hint = false;
};

static std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

static bool is_file(const fs::path& p) { return fs::is_regular_file(p); }
static bool has_ext(const fs::path& p, const std::string& ext) { return lower(p.extension().string()) == ext; }

static void add_candidate(std::map<std::string, Candidate>& out, const fs::path& raw, const std::string& hint, bool launcher = false) {
    std::error_code ec;
    if (!fs::exists(raw, ec) || !fs::is_directory(raw, ec)) return;
    auto canonical = fs::weakly_canonical(raw, ec);
    if (ec) canonical = raw;
    auto key = lower(canonical.string());
    auto [it, inserted] = out.emplace(key, Candidate{canonical});
    it->second.hints.push_back(hint);
    it->second.launcher_hint = it->second.launcher_hint || launcher;
}

static void inspect(Candidate& c) {
    std::error_code ec;
    std::vector<std::pair<fs::path, bool>> roots{{c.root, false}, {c.root / "Data", true}, {c.root / "data", true}};
    std::set<std::string> seen;
    for (const auto& [base, recursive] : roots) {
        if (!fs::is_directory(base, ec)) continue;
        if (!recursive) {
            for (fs::directory_iterator it(base, fs::directory_options::skip_permission_denied, ec), end; it != end && !ec; it.increment(ec)) {
                if (!is_file(it->path())) continue;
                auto key = lower(it->path().string());
                if (!seen.insert(key).second) continue;
                ++c.data_files;
                auto name = lower(it->path().filename().string());
                if (name == "renegade.exe") c.executable = true;
                if (has_ext(it->path(), ".mix")) ++c.mix_count;
            }
            continue;
        }
        fs::recursive_directory_iterator it(base, fs::directory_options::skip_permission_denied, ec), end;
        for (; it != end && !ec; it.increment(ec)) {
            if (!is_file(it->path())) continue;
            auto key = lower(it->path().string());
            if (!seen.insert(key).second) continue;
            ++c.data_files;
            auto name = lower(it->path().filename().string());
            if (name == "always.dat") c.always_dat = true;
            if (name.rfind("00000409.", 0) == 0 || name.rfind("00000409", 0) == 0) c.localized = true;
            if (name == "renegade.exe") c.executable = true;
            if (has_ext(it->path(), ".mix")) ++c.mix_count;
            if (has_ext(it->path(), ".w3d")) ++c.w3d_count;
            if (has_ext(it->path(), ".lsd") || has_ext(it->path(), ".ldd") || has_ext(it->path(), ".lvl")) ++c.map_count;
        }
    }
    if (c.always_dat) c.score += 100;
    if (c.localized) c.score += 80;
    c.score += static_cast<int>(std::min<std::uintmax_t>(c.mix_count * 5, 50));
    if (c.w3d_count) c.score += 20;
    if (c.launcher_hint) c.score += 10;
    if (c.executable) c.score += 5;
}

static void json_string(std::ostream& out, const std::string& s) {
    out << '"';
    for (unsigned char c : s) {
        switch (c) { case '\\': out << "\\\\"; break; case '"': out << "\\\""; break; case '\n': out << "\\n"; break; case '\r': out << "\\r"; break; case '\t': out << "\\t"; break; default: if (c < 0x20) out << "?"; else out << c; }
    }
    out << '"';
}

static void emit(const std::vector<Candidate>& candidates, const fs::path& output) {
    std::ofstream out(output);
    if (!out) throw std::runtime_error("cannot open output: " + output.string());
    out << "{\n  \"schema_version\": 1,\n  \"tool\": \"openw3d-asset-inspect\",\n  \"candidates\": [\n";
    for (std::size_t i = 0; i < candidates.size(); ++i) {
        const auto& c = candidates[i];
        out << "    {\n      \"root\": "; json_string(out, c.root.string());
        out << ",\n      \"score\": " << c.score << ",\n      \"valid\": " << ((c.always_dat || c.localized || c.mix_count || c.w3d_count) ? "true" : "false");
        out << ",\n      \"always_dat\": " << (c.always_dat ? "true" : "false") << ",\n      \"localized_resources\": " << (c.localized ? "true" : "false");
        out << ",\n      \"mix_count\": " << c.mix_count << ",\n      \"w3d_count\": " << c.w3d_count << ",\n      \"map_count\": " << c.map_count << ",\n      \"data_file_count\": " << c.data_files << ",\n      \"retail_executable_present\": " << (c.executable ? "true" : "false") << ",\n      \"hints\": [";
        for (std::size_t h = 0; h < c.hints.size(); ++h) { if (h) out << ", "; json_string(out, c.hints[h]); }
        out << "]\n    }" << (i + 1 == candidates.size() ? "\n" : ",\n");
    }
    out << "  ]\n}\n";
}

int main(int argc, char** argv) {
    try {
        std::vector<fs::path> explicit_roots;
        fs::path output = "asset-inventory.json";
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--output" && i + 1 < argc) output = argv[++i];
            else if (arg == "--root" && i + 1 < argc) explicit_roots.emplace_back(argv[++i]);
            else if (arg == "--help") { std::cout << "Usage: openw3d-asset-inspect [--root PATH]... [--output FILE]\n"; return 0; }
            else explicit_roots.emplace_back(arg);
        }
        std::map<std::string, Candidate> candidates;
        for (const auto& p : explicit_roots) add_candidate(candidates, p, "explicit", false);
#ifdef _WIN32
        const char* program_files = std::getenv("ProgramFiles");
        const char* program_files_x86 = std::getenv("ProgramFiles(x86)");
        std::vector<fs::path> defaults;
        if (program_files) defaults.emplace_back(program_files);
        if (program_files_x86) defaults.emplace_back(program_files_x86);
        for (const auto& pf : defaults) {
            add_candidate(candidates, pf / "Westwood/Renegade", "historical-default");
            add_candidate(candidates, pf / "EA Games/Command & Conquer The First Decade/Command & Conquer Renegade", "tfd-default");
            add_candidate(candidates, pf / "EA Games/Command & Conquer The First Decade/Renegade", "tfd-default");
        }
#endif
        std::vector<Candidate> result;
        for (auto& [_, c] : candidates) { inspect(c); if (c.score > 0) result.push_back(c); }
        std::sort(result.begin(), result.end(), [](const Candidate& a, const Candidate& b) { return a.score > b.score; });
        emit(result, output);
        std::cout << "Inspected " << candidates.size() << " candidate roots; accepted " << result.size() << ".\n";
        for (const auto& c : result) std::cout << c.score << "  " << c.root.string() << " (MIX=" << c.mix_count << ", W3D=" << c.w3d_count << ")\n";
        return 0;
    } catch (const std::exception& e) { std::cerr << "error: " << e.what() << '\n'; return 1; }
}
