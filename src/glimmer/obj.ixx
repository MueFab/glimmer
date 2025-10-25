module;
#include <istream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>
#include <optional>

export module glimmer.obj;

import glimmer.vector;
import glimmer.mesh;

namespace glimmer {
    namespace obj_detail {
        inline void ltrim(std::string& s){
            size_t i=0; while(i<s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i; s.erase(0,i);
        }
        inline void rtrim(std::string& s){
            if (s.empty()) return; size_t i=s.size(); while(i>0 && std::isspace(static_cast<unsigned char>(s[i-1]))) --i; s.erase(i);
        }
        inline void trim(std::string& s){ ltrim(s); rtrim(s); }
        inline void strip_comment(std::string& s){ if (auto pos = s.find('#'); pos != std::string::npos) s.erase(pos); }

        template <Arithmetic T>
        inline void parse_vertex_record(std::istringstream& ls, Mesh<T>& mesh){
            T x{}, y{}, z{}; ls >> x >> y >> z; if (!ls.fail()) mesh.add_vertex(Vector<T,3>{x,y,z});
        }

        inline std::optional<std::size_t> parse_position_index_token(const std::string& token, std::size_t vertex_count){
            if (token.empty()) return std::nullopt;
            std::string idxs = token;
            auto slash = idxs.find('/');
            if (slash != std::string::npos) idxs.erase(slash);
            if (idxs.empty()) return std::nullopt;
            long long idx_val = 0;
            try {
                idx_val = std::stoll(idxs);
            } catch (...) {
                return std::nullopt;
            }
            const long long n = static_cast<long long>(vertex_count);
            long long vi = 0;
            if (idx_val > 0) vi = idx_val - 1; // 1-based
            else if (idx_val < 0) vi = n + idx_val; // negative relative
            else return std::nullopt; // 0 invalid in OBJ
            if (vi < 0 || vi >= n) return std::nullopt;
            return static_cast<std::size_t>(vi);
        }

        template <Arithmetic T>
        inline void triangulate_and_add(Mesh<T>& mesh, const std::vector<std::size_t>& idxs){
            if (idxs.size() < 3) return;
            for (std::size_t i = 2; i < idxs.size(); ++i) mesh.add_triangle(idxs[0], idxs[i-1], idxs[i]);
        }

        template <Arithmetic T>
        inline void parse_face_record(std::istringstream& ls, Mesh<T>& mesh){
            std::vector<std::size_t> face_indices; face_indices.reserve(8);
            std::string vert;
            while (ls >> vert) {
                if (auto oi = parse_position_index_token(vert, mesh.vertex_count())) face_indices.push_back(*oi);
            }
            triangulate_and_add(mesh, face_indices);
        }
    }

    /**
     * @brief Minimal Wavefront OBJ loader.
     * @details Supports:
     *  - Vertex positions: lines starting with 'v' (three components)
     *  - Faces: lines starting with 'f' (triangles/quads/ngons). Triangulates using a fan.
     *  - Indices can be absolute (1-based) or negative (relative to current vertex count).
     *  - Per-vertex formats i, i/tex, i//n, i/tex/n — only the position index is used.
     *  - Comments (#) and blank lines are ignored. Other record types are skipped.
     */
    export template <Arithmetic T>
    [[nodiscard]] Mesh<T> load_obj(std::istream& in) {
        using namespace obj_detail;
        Mesh<T> mesh;
        std::string line;
        while (std::getline(in, line)) {
            strip_comment(line);
            trim(line);
            if (line.empty()) continue;
            std::istringstream ls(line);
            std::string tag; ls >> tag;
            if (tag == "v") { parse_vertex_record(ls, mesh); continue; }
            if (tag == "f") { parse_face_record(ls, mesh); continue; }
            // skip other tags
        }
        return mesh;
    }

    /** Load OBJ from a file path. Throws std::runtime_error if the file cannot be opened. */
    export template <Arithmetic T>
    [[nodiscard]] Mesh<T> load_obj(const std::string& path) {
        std::ifstream f(path);
        if (!f.is_open()) throw std::runtime_error("Failed to open OBJ file: " + path);
        return load_obj<T>(f);
    }
}
