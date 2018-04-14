// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <array>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

#if (defined(__cplusplus) && __cplusplus >= 201703L) || (defined(_HAS_CXX17) && _HAS_CXX17 == 1)
#define FX_GLTF_HAS_CPP_17
#endif

namespace fx
{
namespace base64
{
    namespace detail
    {
        // clang-format off
        constexpr std::array<char, 64> EncodeMap =
        {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
            'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
            'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
            'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
        };

        constexpr std::array<char, 256> DecodeMap =
        {
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
            -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
            -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
            41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        };
        // clang-format on
    } // namespace detail

    inline std::string Encode(std::vector<char> const & bytes)
    {
        const std::size_t length = bytes.size();
        if (length == 0)
        {
            return {};
        }

        std::string out{};
        out.reserve(((length * 4 / 3) + 3) & (~3u)); // round up to nearest 4

        uint32_t val = 0;
        int32_t valb = -6;
        for (const char c : bytes)
        {
            val = (val << 8u) + static_cast<uint8_t>(c);
            valb += 8;
            while (valb >= 0)
            {
                const uint32_t shiftOperand = valb;
                out.push_back(detail::EncodeMap.at((val >> shiftOperand) & 0x3fu));
                valb -= 6;
            }
        }

        if (valb > -6)
        {
            const uint32_t shiftOperand = valb + 8;
            out.push_back(detail::EncodeMap.at(((val << 8u) >> shiftOperand) & 0x3fu));
        }

        while (out.size() % 4 != 0)
        {
            out.push_back('=');
        }

        return out;
    }

    inline bool TryDecode(const std::string & in, std::vector<char> & out)
    {
        out.clear();

        const std::size_t length = in.length();
        if (length == 0)
        {
            return true;
        }

        if (length % 4 != 0)
        {
            return false;
        }

        out.reserve((length / 4) * 3 + 2);

        bool invalid = false;
        uint32_t val = 0;
        int32_t valb = -8;
        for (std::size_t i = 0; i < length; i++)
        {
            const uint8_t c = static_cast<uint8_t>(in[i]);
            const char map = detail::DecodeMap.at(c);
            if (map == -1)
            {
                if (c != '=') // Non base64 character
                {
                    invalid = true;
                }
                else
                {
                    // Padding characters not where they should be
                    const std::size_t remaining = length - i - 1;
                    if (remaining > 1 || (remaining == 1 ? in[i + 1] != '=' : false))
                    {
                        invalid = true;
                    }
                }

                break;
            }

            val = (val << 6u) + map;
            valb += 6;
            if (valb >= 0)
            {
                const uint32_t shiftOperand = valb;
                out.push_back(char((val >> shiftOperand) & 0xffu));
                valb -= 8;
            }
        }

        if (invalid)
        {
            out.clear();
        }

        return !invalid;
    }
} // namespace base64

namespace gltf
{
    class invalid_gltf_document : public std::exception
    {
    public:
        explicit invalid_gltf_document(char const * message) noexcept
            : std::exception(message)
        {
        }

        invalid_gltf_document(char const * message, std::string const & extra)
            : std::exception(CreateMessage(message, extra).c_str())
        {
        }

    private:
        std::string CreateMessage(char const * message, std::string const & extra)
        {
            return std::string(message).append(" : ").append(extra);
        }
    };

    namespace detail
    {
#if defined(FX_GLTF_HAS_CPP_17)
        template <typename TTarget>
        inline void ReadRequiredField(std::string_view key, nlohmann::json const & json, TTarget & target)
#else
        template <typename TKey, typename TTarget>
        inline void ReadRequiredField(TKey && key, nlohmann::json const & json, TTarget & target)
#endif
        {
            const nlohmann::json::const_iterator iter = json.find(key);
            if (iter == json.end())
            {
                throw invalid_gltf_document("Required field not found", std::string(key));
            }

            target = iter->get<TTarget>();
        }

#if defined(FX_GLTF_HAS_CPP_17)
        template <typename TTarget>
        inline void ReadOptionalField(std::string_view key, nlohmann::json const & json, TTarget & target)
#else
        template <typename TKey, typename TTarget>
        inline void ReadOptionalField(TKey && key, nlohmann::json const & json, TTarget & target)
#endif
        {
            const nlohmann::json::const_iterator iter = json.find(key);
            if (iter != json.end())
            {
                target = iter->get<TTarget>();
            }
        }

        template <typename TValue>
        inline void WriteField(std::string const & key, nlohmann::json & json, TValue const & value)
        {
            if (!value.empty())
            {
                json[key] = value;
            }
        }

        template <typename TValue>
        inline void WriteField(std::string const & key, nlohmann::json & json, TValue const & value, TValue const & defaultValue)
        {
            if (value != defaultValue)
            {
                json[key] = value;
            }
        }

        inline std::string GetDocumentRootPath(std::string const & documentFilePath)
        {
            const std::size_t pos = documentFilePath.find_last_of("/\\");
            if (pos != std::string::npos)
            {
                return documentFilePath.substr(0, pos);
            }

            return {};
        }

        inline std::string CreateBufferUriPath(std::string const & documentRootPath, std::string const & bufferUri)
        {
            // Prevent simple forms of path traversal from malicious uri references...
            if (bufferUri.empty() || bufferUri.find("..") != std::string::npos || bufferUri.front() == '/' || bufferUri.front() == '\\')
            {
                throw invalid_gltf_document("Invalid buffer.uri value", bufferUri);
            }

            std::string documentRoot = documentRootPath;
            if (documentRoot.length() > 0)
            {
                if (documentRoot.back() != '/')
                {
                    documentRoot.push_back('/');
                }
            }

            return documentRoot + bufferUri;
        }

        constexpr char const * const mimetypeApplicationOctet = "data:application/octet-stream;base64";
    } // namespace detail

    namespace defaults
    {
        constexpr std::array<float, 16> IdentityMatrix{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
        constexpr std::array<float, 4> IdentityRotation{ 0, 0, 0, 1 };
        constexpr std::array<float, 4> IdentityVec4{ 1, 1, 1, 1 };
        constexpr std::array<float, 3> IdentityVec3{ 1, 1, 1 };
        constexpr std::array<float, 3> NullVec3{ 0, 0, 0 };
        constexpr float IdentityScalar = 1.0f;
        constexpr float FloatSentinal = 9999.75;

        constexpr bool AccessorNormalized = false;

        constexpr float MaterialAlphaCutoff = 0.5f;
        constexpr bool MaterialDoubleSided = false;
    } // namespace defaults

    using Attributes = std::unordered_map<std::string, uint32_t>;

    struct NeverEmpty
    {
        bool empty() const noexcept
        {
            return false;
        }
    };

    struct Accessor
    {
        enum class ComponentType : uint16_t
        {
            None = 0,
            Byte = 5120,
            UnsignedByte = 5121,
            Short = 5122,
            UnsignedShort = 5123,
            UnsignedInt = 5125,
            Float = 5126
        };

        enum class Type : uint8_t
        {
            None,
            Scalar,
            Vec2,
            Vec3,
            Vec4,
            Mat2,
            Mat3,
            Mat4
        };

        struct Sparse
        {
            struct Indices : NeverEmpty
            {
                uint32_t bufferView{};
                uint32_t byteOffset{};
                ComponentType componentType{ ComponentType::None };
            };

            struct Values : NeverEmpty
            {
                uint32_t bufferView{};
                uint32_t byteOffset{};
            };

            int32_t count{};
            Indices indices{};
            Values values{};

            bool empty() const noexcept
            {
                return count == 0;
            }
        };

        int32_t bufferView{ -1 };
        uint32_t byteOffset{};
        uint32_t count{};
        bool normalized{ defaults::AccessorNormalized };

        ComponentType componentType{ ComponentType::None };
        Type type{ Type::None };
        Sparse sparse{};

        std::string name;
        std::vector<float> max{};
        std::vector<float> min{};
    };

    struct Animation
    {
        struct Channel
        {
            struct Target : NeverEmpty
            {
                int32_t node{ -1 };
                std::string path{};
            };

            int32_t sampler{ -1 };
            Target target{};
        };

        struct Sampler
        {
            enum class Type
            {
                Linear,
                Step,
                CubicSpline
            };

            int32_t input{ -1 };
            int32_t output{ -1 };

            Type interpolation{ Sampler::Type::Linear };
        };

        std::string name{};
        std::vector<Channel> channels{};
        std::vector<Sampler> samplers{};
    };

    struct Asset : NeverEmpty
    {
        std::string copyright{};
        std::string generator{};
        std::string minVersion{};
        std::string version{ "2.0" };
    };

    struct Buffer
    {
        uint32_t byteLength{};

        std::string name;
        std::string uri;

        std::vector<char> data{};
    };

    struct BufferView
    {
        enum class TargetType : uint16_t
        {
            None = 0,
            ArrayBuffer = 34962,
            ElementArrayBuffer = 34963
        };

        std::string name;

        int32_t buffer{ -1 };
        uint32_t byteOffset{};
        uint32_t byteLength{};
        uint32_t byteStride{};

        TargetType target{ TargetType::None };
    };

    struct Camera
    {
        enum class Type
        {
            None,
            Orthographic,
            Perspective
        };

        struct Orthographic : NeverEmpty
        {
            float xmag{ defaults::FloatSentinal };
            float ymag{ defaults::FloatSentinal };
            float zfar{ defaults::FloatSentinal };
            float znear{ defaults::FloatSentinal };
        };

        struct Perspective : NeverEmpty
        {
            float aspectRatio{};
            float yfov{};
            float zfar{};
            float znear{};
        };

        std::string name{};
        Type type{ Type::None };

        Orthographic orthographic;
        Perspective perspective;
    };

    struct Image
    {
        int32_t bufferView{};

        std::string name;
        std::string uri;
        std::string mimeType;
    };

    struct Material
    {
        enum class AlphaMode : uint8_t
        {
            Opaque,
            Mask,
            Blend
        };

        struct Texture
        {
            int32_t index{ -1 };
            int32_t texCoord{ -1 };

            bool empty() const noexcept
            {
                return index == -1 && texCoord == -1;
            }
        };

        struct NormalTexture : Texture
        {
            float scale{ defaults::IdentityScalar };
        };

        struct OcclusionTexture : Texture
        {
            float strength{ defaults::IdentityScalar };
        };

        struct PBRMetallicRoughness
        {
            std::array<float, 4> baseColorFactor = { defaults::IdentityVec4 };
            Texture baseColorTexture;

            float roughnessFactor{ defaults::IdentityScalar };
            float metallicFactor{ defaults::IdentityScalar };
            Texture metallicRoughnessTexture;

            bool empty() const noexcept
            {
                return baseColorTexture.empty() && metallicRoughnessTexture.empty() && metallicFactor == 1.0f && roughnessFactor == 1.0f && baseColorFactor == defaults::IdentityVec4;
            }
        };

        float alphaCutoff{ defaults::MaterialAlphaCutoff };
        AlphaMode alphaMode{ AlphaMode::Opaque };

        bool doubleSided{ defaults::MaterialDoubleSided };

        NormalTexture normalTexture;
        OcclusionTexture occlusionTexture;
        PBRMetallicRoughness pbrMetallicRoughness;

        Texture emissiveTexture;
        std::array<float, 3> emissiveFactor = { defaults::NullVec3 };

        std::string name;
    };

    struct Primitive
    {
        enum class Mode : uint8_t
        {
            Points = 0,
            Lines = 1,
            LineLoop = 2,
            LineStrip = 3,
            Triangles = 4,
            TriangleStrip = 5,
            TriangleFan = 6
        };

        int32_t indices{ -1 };
        int32_t material{ -1 };

        Mode mode{ Mode::Triangles };

        Attributes attributes{};
        std::vector<Attributes> targets{};
    };

    struct Mesh
    {
        std::string name;

        std::vector<float> weights{};
        std::vector<Primitive> primitives{};
    };

    struct Node
    {
        std::string name;

        int32_t camera{ -1 };
        int32_t mesh{ -1 };
        int32_t skin{ -1 };

        std::array<float, 16> matrix{ defaults::IdentityMatrix };
        std::array<float, 4> rotation{ defaults::IdentityRotation };
        std::array<float, 3> scale{ defaults::IdentityVec3 };
        std::array<float, 3> translation{ defaults::NullVec3 };

        std::vector<int32_t> children{};
        std::vector<float> weights{};
    };

    struct Sampler
    {
        enum class MagFilter : uint16_t
        {
            None,
            Nearest = 9728,
            Linear = 9729
        };

        enum class MinFilter : uint16_t
        {
            None,
            Nearest = 9728,
            Linear = 9729,
            NearestMipMapNearest = 9984,
            LinearMipMapNearest = 9985,
            NearestMipMapLinear = 9986,
            LinearMipMapLinear = 9987
        };

        enum class WrappingMode : uint16_t
        {
            ClampToEdge = 33071,
            MirroredRepeat = 33648,
            Repeat = 10497
        };

        std::string name;

        MagFilter magFilter{ MagFilter::None };
        MinFilter minFilter{ MinFilter::None };

        WrappingMode wrapS{ WrappingMode::Repeat };
        WrappingMode wrapT{ WrappingMode::Repeat };

        bool empty() const noexcept
        {
            return name.empty() && magFilter == MagFilter::None && minFilter == MinFilter::None && wrapS == WrappingMode::Repeat && wrapT == WrappingMode::Repeat;
        }
    };

    struct Scene
    {
        std::string name;

        std::vector<uint32_t> nodes{};
    };

    struct Skin
    {
        int32_t inverseBindMatrices{ -1 };
        int32_t skeleton{ -1 };

        std::vector<uint32_t> joints{};
        std::string name;
    };

    struct Texture
    {
        std::string name;

        int32_t sampler{ -1 };
        int32_t source{ -1 };
    };

    struct Document
    {
        Asset asset;

        std::vector<Accessor> accessors{};
        std::vector<Animation> animations{};
        std::vector<Buffer> buffers{};
        std::vector<BufferView> bufferViews{};
        std::vector<Camera> cameras{};
        std::vector<Image> images{};
        std::vector<Material> materials{};
        std::vector<Mesh> meshes{};
        std::vector<Node> nodes{};
        std::vector<Sampler> samplers{};
        std::vector<Scene> scenes{};
        std::vector<Skin> skins{};
        std::vector<Texture> textures{};

        int32_t scene{ -1 };

        static Document Create(nlohmann::json const & json, std::string const & bufferRootPath)
        {
            Document document = json;

            for (auto & buffer : document.buffers)
            {
                if (buffer.byteLength > 0 && !buffer.uri.empty())
                {
                    if (buffer.uri.find(detail::mimetypeApplicationOctet) == std::string::npos)
                    {
                        std::ifstream fileData(detail::CreateBufferUriPath(bufferRootPath, buffer.uri), std::ios::binary);
                        if (!fileData.good())
                        {
                            throw invalid_gltf_document("Invalid buffer.uri value", buffer.uri);
                        }

                        buffer.data.resize(buffer.byteLength);
                        fileData.read(&buffer.data[0], buffer.byteLength);
                    }
                    else
                    {
                        bool success = base64::TryDecode(buffer.uri.substr(std::char_traits<char>::length(detail::mimetypeApplicationOctet) + 1), buffer.data);
                        if (!success)
                        {
                            throw invalid_gltf_document("Invalid buffer.uri value", "bad base64");
                        }
                    }
                }
            }

            return document;
        }
    };

    inline void from_json(nlohmann::json const & json, Accessor::Type & accessorType)
    {
        std::string type = json.get<std::string>();
        if (type == "SCALAR")
        {
            accessorType = Accessor::Type::Scalar;
        }
        else if (type == "VEC2")
        {
            accessorType = Accessor::Type::Vec2;
        }
        else if (type == "VEC3")
        {
            accessorType = Accessor::Type::Vec3;
        }
        else if (type == "VEC4")
        {
            accessorType = Accessor::Type::Vec4;
        }
        else if (type == "MAT2")
        {
            accessorType = Accessor::Type::Mat2;
        }
        else if (type == "MAT3")
        {
            accessorType = Accessor::Type::Mat3;
        }
        else if (type == "MAT4")
        {
            accessorType = Accessor::Type::Mat4;
        }
        else
        {
            throw invalid_gltf_document("Unknown accessor.type value", type);
        }
    }

    inline void from_json(nlohmann::json const & json, Accessor::Sparse::Values & values)
    {
        detail::ReadRequiredField("bufferView", json, values.bufferView);

        detail::ReadOptionalField("byteOffset", json, values.byteOffset);
    }

    inline void from_json(nlohmann::json const & json, Accessor::Sparse::Indices & indices)
    {
        detail::ReadRequiredField("bufferView", json, indices.bufferView);
        detail::ReadRequiredField("componentType", json, indices.componentType);

        detail::ReadOptionalField("byteOffset", json, indices.byteOffset);
    }

    inline void from_json(nlohmann::json const & json, Accessor::Sparse & sparse)
    {
        detail::ReadRequiredField("count", json, sparse.count);
        detail::ReadRequiredField("indices", json, sparse.indices);
        detail::ReadRequiredField("values", json, sparse.values);
    }

    inline void from_json(nlohmann::json const & json, Accessor & accessor)
    {
        detail::ReadRequiredField("componentType", json, accessor.componentType);
        detail::ReadRequiredField("count", json, accessor.count);
        detail::ReadRequiredField("type", json, accessor.type);

        detail::ReadOptionalField("bufferView", json, accessor.bufferView);
        detail::ReadOptionalField("byteOffset", json, accessor.byteOffset);
        detail::ReadOptionalField("max", json, accessor.max);
        detail::ReadOptionalField("min", json, accessor.min);
        detail::ReadOptionalField("name", json, accessor.name);
        detail::ReadOptionalField("normalized", json, accessor.normalized);
        detail::ReadOptionalField("sparse", json, accessor.sparse);
    }

    inline void from_json(nlohmann::json const & json, Animation::Channel::Target & animationChannelTarget)
    {
        detail::ReadRequiredField("path", json, animationChannelTarget.path);

        detail::ReadOptionalField("node", json, animationChannelTarget.node);
    }

    inline void from_json(nlohmann::json const & json, Animation::Channel & animationChannel)
    {
        detail::ReadRequiredField("sampler", json, animationChannel.sampler);
        detail::ReadRequiredField("target", json, animationChannel.target);
    }

    inline void from_json(nlohmann::json const & json, Animation::Sampler::Type & animationSamplerType)
    {
        std::string type = json.get<std::string>();
        if (type == "LINEAR")
        {
            animationSamplerType = Animation::Sampler::Type::Linear;
        }
        else if (type == "STEP")
        {
            animationSamplerType = Animation::Sampler::Type::Step;
        }
        else if (type == "CUBICSPLINE")
        {
            animationSamplerType = Animation::Sampler::Type::CubicSpline;
        }
        else
        {
            throw invalid_gltf_document("Unknown animation.sampler.interpolation value", type);
        }
    }

    inline void from_json(nlohmann::json const & json, Animation::Sampler & animationSampler)
    {
        detail::ReadRequiredField("input", json, animationSampler.input);
        detail::ReadRequiredField("output", json, animationSampler.output);

        detail::ReadOptionalField("interpolation", json, animationSampler.interpolation);
    }

    inline void from_json(nlohmann::json const & json, Animation & animation)
    {
        detail::ReadRequiredField("channels", json, animation.channels);
        detail::ReadRequiredField("samplers", json, animation.samplers);

        detail::ReadOptionalField("name", json, animation.name);
    }

    inline void from_json(nlohmann::json const & json, Asset & asset)
    {
        detail::ReadRequiredField("version", json, asset.version);
        detail::ReadOptionalField("copyright", json, asset.copyright);
        detail::ReadOptionalField("generator", json, asset.generator);
        detail::ReadOptionalField("minVersion", json, asset.minVersion);
    }

    inline void from_json(nlohmann::json const & json, Buffer & buffer)
    {
        detail::ReadRequiredField("byteLength", json, buffer.byteLength);

        detail::ReadOptionalField("name", json, buffer.name);
        detail::ReadOptionalField("uri", json, buffer.uri);
    }

    inline void from_json(nlohmann::json const & json, BufferView & bufferView)
    {
        detail::ReadRequiredField("buffer", json, bufferView.buffer);
        detail::ReadRequiredField("byteLength", json, bufferView.byteLength);

        detail::ReadOptionalField("byteOffset", json, bufferView.byteOffset);
        detail::ReadOptionalField("byteStride", json, bufferView.byteStride);
        detail::ReadOptionalField("name", json, bufferView.name);
        detail::ReadOptionalField("target", json, bufferView.target);
    }

    inline void from_json(nlohmann::json const & json, Camera::Type & cameraType)
    {
        std::string type = json.get<std::string>();
        if (type == "orthographic")
        {
            cameraType = Camera::Type::Orthographic;
        }
        else if (type == "perspective")
        {
            cameraType = Camera::Type::Perspective;
        }
        else
        {
            throw invalid_gltf_document("Unknown camera.type value", type);
        }
    }

    inline void from_json(nlohmann::json const & json, Camera::Orthographic & camera)
    {
        detail::ReadRequiredField("xmag", json, camera.xmag);
        detail::ReadRequiredField("ymag", json, camera.ymag);
        detail::ReadRequiredField("zfar", json, camera.zfar);
        detail::ReadRequiredField("znear", json, camera.znear);
    }

    inline void from_json(nlohmann::json const & json, Camera::Perspective & camera)
    {
        detail::ReadRequiredField("yfov", json, camera.yfov);
        detail::ReadRequiredField("znear", json, camera.znear);

        detail::ReadOptionalField("aspectRatio", json, camera.aspectRatio);
        detail::ReadOptionalField("zfar", json, camera.zfar);
    }

    inline void from_json(nlohmann::json const & json, Camera & camera)
    {
        detail::ReadRequiredField("type", json, camera.type);

        detail::ReadOptionalField("name", json, camera.name);

        if (camera.type == Camera::Type::Perspective)
        {
            detail::ReadRequiredField("perspective", json, camera.perspective);
        }
        else if (camera.type == Camera::Type::Orthographic)
        {
            detail::ReadRequiredField("orthographic", json, camera.orthographic);
        }
    }

    inline void from_json(nlohmann::json const & json, Image & image)
    {
        detail::ReadOptionalField("bufferView", json, image.bufferView);
        detail::ReadOptionalField("mimeType", json, image.mimeType);
        detail::ReadOptionalField("name", json, image.name);
        detail::ReadOptionalField("uri", json, image.uri);
    }

    inline void from_json(nlohmann::json const & json, Material::AlphaMode & materialAlphaMode)
    {
        std::string alphaMode = json.get<std::string>();
        if (alphaMode == "OPAQUE")
        {
            materialAlphaMode = Material::AlphaMode::Opaque;
        }
        else if (alphaMode == "MASK")
        {
            materialAlphaMode = Material::AlphaMode::Mask;
        }
        else if (alphaMode == "BLEND")
        {
            materialAlphaMode = Material::AlphaMode::Blend;
        }
        else
        {
            throw invalid_gltf_document("Unknown material.alphaMode value", alphaMode);
        }
    }

    inline void from_json(nlohmann::json const & json, Material::Texture & materialTexture)
    {
        detail::ReadRequiredField("index", json, materialTexture.index);
        detail::ReadOptionalField("texCoord", json, materialTexture.texCoord);
    }

    inline void from_json(nlohmann::json const & json, Material::NormalTexture & materialTexture)
    {
        from_json(json, static_cast<Material::Texture &>(materialTexture));
        detail::ReadOptionalField("scale", json, materialTexture.scale);
    }

    inline void from_json(nlohmann::json const & json, Material::OcclusionTexture & materialTexture)
    {
        from_json(json, static_cast<Material::Texture &>(materialTexture));
        detail::ReadOptionalField("strength", json, materialTexture.strength);
    }

    inline void from_json(nlohmann::json const & json, Material::PBRMetallicRoughness & pbrMetallicRoughness)
    {
        detail::ReadOptionalField("baseColorFactor", json, pbrMetallicRoughness.baseColorFactor);
        detail::ReadOptionalField("baseColorTexture", json, pbrMetallicRoughness.baseColorTexture);
        detail::ReadOptionalField("metallicFactor", json, pbrMetallicRoughness.metallicFactor);
        detail::ReadOptionalField("metallicRoughnessTexture", json, pbrMetallicRoughness.metallicRoughnessTexture);
        detail::ReadOptionalField("roughnessFactor", json, pbrMetallicRoughness.roughnessFactor);
    }

    inline void from_json(nlohmann::json const & json, Material & material)
    {
        detail::ReadOptionalField("alphaMode", json, material.alphaMode);
        detail::ReadOptionalField("alphaCutoff", json, material.alphaCutoff);
        detail::ReadOptionalField("doubleSided", json, material.doubleSided);
        detail::ReadOptionalField("emissiveFactor", json, material.emissiveFactor);
        detail::ReadOptionalField("emissiveTexture", json, material.emissiveTexture);
        detail::ReadOptionalField("name", json, material.name);
        detail::ReadOptionalField("normalTexture", json, material.normalTexture);
        detail::ReadOptionalField("occlusionTexture", json, material.occlusionTexture);
        detail::ReadOptionalField("pbrMetallicRoughness", json, material.pbrMetallicRoughness);
    }

    inline void from_json(nlohmann::json const & json, Mesh & mesh)
    {
        detail::ReadRequiredField("primitives", json, mesh.primitives);

        detail::ReadOptionalField("name", json, mesh.name);
        detail::ReadOptionalField("weights", json, mesh.weights);
    }

    inline void from_json(nlohmann::json const & json, Node & node)
    {
        detail::ReadOptionalField("camera", json, node.camera);
        detail::ReadOptionalField("children", json, node.children);
        detail::ReadOptionalField("matrix", json, node.matrix);
        detail::ReadOptionalField("mesh", json, node.mesh);
        detail::ReadOptionalField("name", json, node.name);
        detail::ReadOptionalField("rotation", json, node.rotation);
        detail::ReadOptionalField("scale", json, node.scale);
        detail::ReadOptionalField("skin", json, node.skin);
        detail::ReadOptionalField("translation", json, node.translation);
    }

    inline void from_json(nlohmann::json const & json, Primitive & primitive)
    {
        detail::ReadRequiredField("attributes", json, primitive.attributes);

        detail::ReadOptionalField("indices", json, primitive.indices);
        detail::ReadOptionalField("material", json, primitive.material);
        detail::ReadOptionalField("mode", json, primitive.mode);
        detail::ReadOptionalField("targets", json, primitive.targets);
    }

    inline void from_json(nlohmann::json const & json, Sampler & sampler)
    {
        detail::ReadOptionalField("magFilter", json, sampler.magFilter);
        detail::ReadOptionalField("minFilter", json, sampler.minFilter);
        detail::ReadOptionalField("name", json, sampler.name);
        detail::ReadOptionalField("wrapS", json, sampler.wrapS);
        detail::ReadOptionalField("wrapT", json, sampler.wrapT);
    }

    inline void from_json(nlohmann::json const & json, Scene & scene)
    {
        detail::ReadOptionalField("name", json, scene.name);
        detail::ReadOptionalField("nodes", json, scene.nodes);
    }

    inline void from_json(nlohmann::json const & json, Skin & skin)
    {
        detail::ReadRequiredField("joints", json, skin.joints);

        detail::ReadOptionalField("inverseBindMatrices", json, skin.inverseBindMatrices);
        detail::ReadOptionalField("name", json, skin.name);
        detail::ReadOptionalField("skeleton", json, skin.skeleton);
    }

    inline void from_json(nlohmann::json const & json, Texture & texture)
    {
        detail::ReadOptionalField("name", json, texture.name);
        detail::ReadOptionalField("sampler", json, texture.sampler);
        detail::ReadOptionalField("source", json, texture.source);
    }

    inline void from_json(nlohmann::json const & json, Document & document)
    {
        detail::ReadRequiredField("asset", json, document.asset);

        detail::ReadOptionalField("accessors", json, document.accessors);
        detail::ReadOptionalField("animations", json, document.animations);
        detail::ReadOptionalField("buffers", json, document.buffers);
        detail::ReadOptionalField("bufferViews", json, document.bufferViews);
        detail::ReadOptionalField("cameras", json, document.cameras);
        detail::ReadOptionalField("materials", json, document.materials);
        detail::ReadOptionalField("meshes", json, document.meshes);
        detail::ReadOptionalField("nodes", json, document.nodes);
        detail::ReadOptionalField("images", json, document.images);
        detail::ReadOptionalField("samplers", json, document.samplers);
        detail::ReadOptionalField("scene", json, document.scene);
        detail::ReadOptionalField("scenes", json, document.scenes);
        detail::ReadOptionalField("skins", json, document.skins);
        detail::ReadOptionalField("textures", json, document.textures);
    }

    inline void to_json(nlohmann::json & json, Accessor::ComponentType const & accessorComponentType)
    {
        if (accessorComponentType == Accessor::ComponentType::None)
        {
            throw invalid_gltf_document("Unknown accessor.componentType value");
        }

        json = static_cast<uint16_t>(accessorComponentType);
    }

    inline void to_json(nlohmann::json & json, Accessor::Type const & accessorType)
    {
        switch (accessorType)
        {
        case Accessor::Type::Scalar:
            json = "SCALAR";
            break;
        case Accessor::Type::Vec2:
            json = "VEC2";
            break;
        case Accessor::Type::Vec3:
            json = "VEC3";
            break;
        case Accessor::Type::Vec4:
            json = "VEC4";
            break;
        case Accessor::Type::Mat2:
            json = "MAT2";
            break;
        case Accessor::Type::Mat3:
            json = "MAT3";
            break;
        case Accessor::Type::Mat4:
            json = "MAT4";
            break;
        default:
            throw invalid_gltf_document("Unknown accessor.type value");
        }
    }

    inline void to_json(nlohmann::json & json, Accessor::Sparse::Values const & values)
    {
        detail::WriteField("bufferView", json, values.bufferView, static_cast<uint32_t>(-1));
        detail::WriteField("byteOffset", json, values.byteOffset, {});
    }

    inline void to_json(nlohmann::json & json, Accessor::Sparse::Indices const & indices)
    {
        detail::WriteField("componentType", json, indices.componentType, Accessor::ComponentType::None);
        detail::WriteField("bufferView", json, indices.bufferView, static_cast<uint32_t>(-1));
        detail::WriteField("byteOffset", json, indices.byteOffset, {});
    }

    inline void to_json(nlohmann::json & json, Accessor::Sparse const & sparse)
    {
        detail::WriteField("count", json, sparse.count, -1);
        detail::WriteField("indices", json, sparse.indices);
        detail::WriteField("values", json, sparse.values);
    }

    inline void to_json(nlohmann::json & json, Accessor const & accessor)
    {
        detail::WriteField("bufferView", json, accessor.bufferView, -1);
        detail::WriteField("byteOffset", json, accessor.byteOffset, {});
        detail::WriteField("componentType", json, accessor.componentType, Accessor::ComponentType::None);
        detail::WriteField("count", json, accessor.count, {});
        detail::WriteField("max", json, accessor.max);
        detail::WriteField("min", json, accessor.min);
        detail::WriteField("name", json, accessor.name);
        detail::WriteField("normalized", json, accessor.normalized, false);
        detail::WriteField("sparse", json, accessor.sparse);
        detail::WriteField("type", json, accessor.type, Accessor::Type::None);
    }

    inline void to_json(nlohmann::json & json, Animation::Channel::Target const & animationChannelTarget)
    {
        detail::WriteField("node", json, animationChannelTarget.node, -1);
        detail::WriteField("path", json, animationChannelTarget.path);
    }

    inline void to_json(nlohmann::json & json, Animation::Channel const & animationChannel)
    {
        detail::WriteField("sampler", json, animationChannel.sampler, -1);
        detail::WriteField("target", json, animationChannel.target);
    }

    inline void to_json(nlohmann::json & json, Animation::Sampler::Type const & animationSamplerType)
    {
        switch (animationSamplerType)
        {
        case Animation::Sampler::Type::Linear:
            json = "LINEAR";
            break;
        case Animation::Sampler::Type::Step:
            json = "STEP";
            break;
        case Animation::Sampler::Type::CubicSpline:
            json = "CUBICSPLINE";
            break;
        }
    }

    inline void to_json(nlohmann::json & json, Animation::Sampler const & animationSampler)
    {
        detail::WriteField("input", json, animationSampler.input, -1);
        detail::WriteField("interpolation", json, animationSampler.interpolation, Animation::Sampler::Type::Linear);
        detail::WriteField("output", json, animationSampler.output, -1);
    }

    inline void to_json(nlohmann::json & json, Animation const & animation)
    {
        detail::WriteField("channels", json, animation.channels);
        detail::WriteField("name", json, animation.name);
        detail::WriteField("samplers", json, animation.samplers);
    }

    inline void to_json(nlohmann::json & json, Asset const & asset)
    {
        detail::WriteField("copyright", json, asset.copyright);
        detail::WriteField("generator", json, asset.generator);
        detail::WriteField("minVersion", json, asset.minVersion);
        detail::WriteField("version", json, asset.version);
    }

    inline void to_json(nlohmann::json & json, Buffer const & buffer)
    {
        detail::WriteField("byteLength", json, buffer.byteLength, {});
        //detail::WriteOptionalField("data", json, buffer.data);
        detail::WriteField("name", json, buffer.name);
        detail::WriteField("uri", json, buffer.uri);
    }

    inline void to_json(nlohmann::json & json, BufferView const & bufferView)
    {
        detail::WriteField("buffer", json, bufferView.buffer, -1);
        detail::WriteField("byteLength", json, bufferView.byteLength, {});
        detail::WriteField("byteOffset", json, bufferView.byteOffset, {});
        detail::WriteField("byteStride", json, bufferView.byteStride, {});
        detail::WriteField("name", json, bufferView.name);
        detail::WriteField("target", json, bufferView.target, BufferView::TargetType::None);
    }

    inline void to_json(nlohmann::json & json, Camera::Type const & cameraType)
    {
        switch (cameraType)
        {
        case Camera::Type::Orthographic:
            json = "orthographic";
            break;
        case Camera::Type::Perspective:
            json = "perspective";
            break;
        default:
            throw invalid_gltf_document("Unknown camera.type value");
        }
    }

    inline void to_json(nlohmann::json & json, Camera::Orthographic const & camera)
    {
        detail::WriteField("xmag", json, camera.xmag, defaults::FloatSentinal);
        detail::WriteField("ymag", json, camera.ymag, defaults::FloatSentinal);
        detail::WriteField("zfar", json, camera.zfar, defaults::FloatSentinal);
        detail::WriteField("znear", json, camera.znear, defaults::FloatSentinal);
    }

    inline void to_json(nlohmann::json & json, Camera::Perspective const & camera)
    {
        detail::WriteField("aspectRatio", json, camera.aspectRatio, {});
        detail::WriteField("yfov", json, camera.yfov, {});
        detail::WriteField("zfar", json, camera.zfar, {});
        detail::WriteField("znear", json, camera.znear, {});
    }

    inline void to_json(nlohmann::json & json, Camera const & camera)
    {
        detail::WriteField("name", json, camera.name);
        detail::WriteField("type", json, camera.type, Camera::Type::None);

        if (camera.type == Camera::Type::Perspective)
        {
            detail::WriteField("perspective", json, camera.perspective);
        }
        else if (camera.type == Camera::Type::Orthographic)
        {
            detail::WriteField("orthographic", json, camera.orthographic);
        }
    }

    inline void to_json(nlohmann::json & json, Image const & image)
    {
        detail::WriteField("bufferView", json, image.bufferView, {});
        detail::WriteField("mimeType", json, image.mimeType);
        detail::WriteField("name", json, image.name);
        detail::WriteField("uri", json, image.uri);
    }

    inline void to_json(nlohmann::json & json, Material::AlphaMode const & materialAlphaMode)
    {
        switch (materialAlphaMode)
        {
        case Material::AlphaMode::Opaque:
            json = "OPAQUE";
            break;
        case Material::AlphaMode::Mask:
            json = "MASK";
            break;
        case Material::AlphaMode::Blend:
            json = "BLEND";
            break;
        }
    }

    inline void to_json(nlohmann::json & json, Material::Texture const & materialTexture)
    {
        detail::WriteField("index", json, materialTexture.index, -1);
        detail::WriteField("texCoord", json, materialTexture.texCoord, -1);
    }

    inline void to_json(nlohmann::json & json, Material::NormalTexture const & materialTexture)
    {
        to_json(json, static_cast<Material::Texture const &>(materialTexture));
        detail::WriteField("scale", json, materialTexture.scale, defaults::IdentityScalar);
    }

    inline void to_json(nlohmann::json & json, Material::OcclusionTexture const & materialTexture)
    {
        to_json(json, static_cast<Material::Texture const &>(materialTexture));
        detail::WriteField("strength", json, materialTexture.strength, defaults::IdentityScalar);
    }

    inline void to_json(nlohmann::json & json, Material::PBRMetallicRoughness const & pbrMetallicRoughness)
    {
        detail::WriteField("baseColorFactor", json, pbrMetallicRoughness.baseColorFactor, defaults::IdentityVec4);
        detail::WriteField("baseColorTexture", json, pbrMetallicRoughness.baseColorTexture);
        detail::WriteField("metallicFactor", json, pbrMetallicRoughness.metallicFactor, defaults::IdentityScalar);
        detail::WriteField("metallicRoughnessTexture", json, pbrMetallicRoughness.metallicRoughnessTexture);
        detail::WriteField("roughnessFactor", json, pbrMetallicRoughness.roughnessFactor, defaults::IdentityScalar);
    }

    inline void to_json(nlohmann::json & json, Material const & material)
    {
        detail::WriteField("alphaCutoff", json, material.alphaCutoff, defaults::MaterialAlphaCutoff);
        detail::WriteField("alphaMode", json, material.alphaMode, Material::AlphaMode::Opaque);
        detail::WriteField("doubleSided", json, material.doubleSided, defaults::MaterialDoubleSided);
        detail::WriteField("emissiveTexture", json, material.emissiveTexture);
        detail::WriteField("emissiveFactor", json, material.emissiveFactor, defaults::NullVec3);
        detail::WriteField("name", json, material.name);
        detail::WriteField("normalTexture", json, material.normalTexture);
        detail::WriteField("occlusionTexture", json, material.occlusionTexture);
        detail::WriteField("pbrMetallicRoughness", json, material.pbrMetallicRoughness);
    }

    inline void to_json(nlohmann::json & json, Mesh const & mesh)
    {
        detail::WriteField("name", json, mesh.name);
        detail::WriteField("primitives", json, mesh.primitives);
        detail::WriteField("weights", json, mesh.weights);
    }

    inline void to_json(nlohmann::json & json, Node const & node)
    {
        detail::WriteField("camera", json, node.camera, -1);
        detail::WriteField("children", json, node.children);
        detail::WriteField("matrix", json, node.matrix, defaults::IdentityMatrix);
        detail::WriteField("mesh", json, node.mesh, -1);
        detail::WriteField("name", json, node.name);
        detail::WriteField("rotation", json, node.rotation, defaults::IdentityRotation);
        detail::WriteField("scale", json, node.scale, defaults::IdentityVec3);
        detail::WriteField("skin", json, node.skin, -1);
        detail::WriteField("translation", json, node.translation, defaults::NullVec3);
        detail::WriteField("weights", json, node.weights);
    }

    inline void to_json(nlohmann::json & json, Primitive const & primitive)
    {
        detail::WriteField("attributes", json, primitive.attributes);
        detail::WriteField("indices", json, primitive.indices, -1);
        detail::WriteField("material", json, primitive.material, -1);
        detail::WriteField("mode", json, primitive.mode, Primitive::Mode::Triangles);
        detail::WriteField("targets", json, primitive.targets);
    }

    inline void to_json(nlohmann::json & json, Sampler const & sampler)
    {
        detail::WriteField("name", json, sampler.name);
        detail::WriteField("magFilter", json, sampler.magFilter, Sampler::MagFilter::None);
        detail::WriteField("minFilter", json, sampler.minFilter, Sampler::MinFilter::None);
        detail::WriteField("wrapS", json, sampler.wrapS, Sampler::WrappingMode::Repeat);
        detail::WriteField("wrapT", json, sampler.wrapT, Sampler::WrappingMode::Repeat);
    }

    inline void to_json(nlohmann::json & json, Scene const & scene)
    {
        detail::WriteField("name", json, scene.name);
        detail::WriteField("nodes", json, scene.nodes);
    }

    inline void to_json(nlohmann::json & json, Skin const & skin)
    {
        detail::WriteField("inverseBindMatrices", json, skin.inverseBindMatrices, -1);
        detail::WriteField("name", json, skin.name);
        detail::WriteField("skeleton", json, skin.skeleton, -1);
        detail::WriteField("joints", json, skin.joints);
    }

    inline void to_json(nlohmann::json & json, Texture const & texture)
    {
        detail::WriteField("name", json, texture.name);
        detail::WriteField("sampler", json, texture.sampler, -1);
        detail::WriteField("source", json, texture.source, -1);
    }

    inline void to_json(nlohmann::json & json, Document const & document)
    {
        detail::WriteField("accessors", json, document.accessors);
        detail::WriteField("animations", json, document.animations);
        detail::WriteField("asset", json, document.asset);
        detail::WriteField("buffers", json, document.buffers);
        detail::WriteField("bufferViews", json, document.bufferViews);
        detail::WriteField("cameras", json, document.cameras);
        detail::WriteField("images", json, document.images);
        detail::WriteField("materials", json, document.materials);
        detail::WriteField("meshes", json, document.meshes);
        detail::WriteField("nodes", json, document.nodes);
        detail::WriteField("samplers", json, document.samplers);
        detail::WriteField("scene", json, document.scene, -1);
        detail::WriteField("scenes", json, document.scenes);
        detail::WriteField("skins", json, document.skins);
        detail::WriteField("textures", json, document.textures);
    }

    inline Document LoadFromText(std::string const & documentFilePath)
    {
        nlohmann::json json;
        {
            std::ifstream file(documentFilePath);
            if (!file.is_open())
            {
                throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory));
            }

            file >> json;
        }

        try
        {
            return Document::Create(json, detail::GetDocumentRootPath(documentFilePath));
        }
        catch (invalid_gltf_document &)
        {
            throw;
        }
        catch (std::exception &)
        {
            std::throw_with_nested(invalid_gltf_document("Invalid glTF document. See nested exception for details."));
        }
    }

    inline void SaveAsText(Document const & document, std::string const & documentFilePath)
    {
        nlohmann::json json = document;

        std::ofstream file(documentFilePath);
        if (!file.is_open())
        {
            throw std::system_error(std::make_error_code(std::errc::io_error));
        }

        file << json.dump(2);
        file.flush();
    }

} // namespace gltf
} // namespace fx

#undef FX_GLTF_HAS_CPP_17
