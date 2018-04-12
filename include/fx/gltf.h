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
#include <vector>

#include <nlohmann/json.hpp>

#if (defined(__cplusplus) && __cplusplus >= 201703L) || (defined(_HAS_CXX17) && _HAS_CXX17 == 1)
#define FX_GLTF_HAS_CPP_17
#endif

namespace fx
{
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
        static void ReadRequiredField(std::string_view key, nlohmann::json const & json, TTarget & target)
#else
        template <typename TKey, typename TTarget>
        static void ReadRequiredField(TKey && key, nlohmann::json const & json, TTarget & target)
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
        static void ReadOptionalField(std::string_view key, nlohmann::json const & json, TTarget & target)
#else
        template <typename TKey, typename TTarget>
        static void ReadOptionalField(TKey && key, nlohmann::json const & json, TTarget & target)
#endif
        {
            const nlohmann::json::const_iterator iter = json.find(key);
            if (iter != json.end())
            {
                target = iter->get<TTarget>();
            }
        }

        template <typename TValue>
        static void WriteField(std::string const & key, nlohmann::json & json, TValue const & value)
        {
            if (!value.empty())
            {
                json[key] = value;
            }
        }

        template <typename TValue>
        static void WriteField(std::string const & key, nlohmann::json & json, TValue const & value, TValue const & defaultValue)
        {
            if (value != defaultValue)
            {
                json[key] = value;
            }
        }

        static std::string GetDocumentRootPath(std::string const & documentFilePath)
        {
            const std::size_t pos = documentFilePath.find_last_of("/\\");
            if (pos != std::string::npos)
            {
                return documentFilePath.substr(0, pos);
            }

            return {};
        }

        static std::string CreateBufferUriPath(std::string const & documentRootPath, std::string const & bufferUri)
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

        std::string name;

        int32_t bufferView{ -1 };
        uint32_t byteOffset{};
        uint32_t count{};

        Type type{ Type::None };
        ComponentType componentType{ ComponentType::None };

        std::vector<float> max{};
        std::vector<float> min{};

        bool normalized{ defaults::AccessorNormalized };
    };

    struct Animation
    {
        struct Channel
        {
            struct Target
            {
                int32_t node{ -1 };
                std::string path{};

                bool empty() const noexcept
                {
                    return false;
                }
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
            int32_t ouput{ -1 };

            Type interpolation{ Sampler::Type::Linear };
        };

        std::string name{};
        std::vector<Channel> channels{};
        std::vector<Sampler> samplers{};

        bool empty() const noexcept
        {
            return false;
        }
    };

    struct Asset
    {
        std::string copyright;
        std::string generator{ "fx-gltf" };
        std::string minVersion;
        std::string version{ "2.0" };

        bool empty() const noexcept
        {
            return false;
        }
    };

    struct Buffer
    {
        std::string name;

        std::string uri;
        uint32_t byteLength{};

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
        struct Orthographic
        {
            float xmag{ defaults::FloatSentinal };
            float ymag{ defaults::FloatSentinal };
            float zfar{ defaults::FloatSentinal };
            float znear{ defaults::FloatSentinal };

            bool empty() const noexcept
            {
                return false;
            }
        };

        struct Perspective
        {
            float aspectRatio{};
            float yfov{};
            float zfar{};
            float znear{};

            bool empty() const noexcept
            {
                return false;
            }
        };

        std::string name;
        std::string type; // TODO Make enumeration

        Orthographic orthographic;
        Perspective perspective;
    };

    struct Image
    {
        std::string name;

        std::string uri;

        std::string mimeType;
        int32_t bufferView{};
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

        std::string name;

        PBRMetallicRoughness pbrMetallicRoughness;

        NormalTexture normalTexture;
        OcclusionTexture occlusionTexture;

        Texture emissiveTexture;
        std::array<float, 3> emissiveFactor = { defaults::NullVec3 };

        AlphaMode alphaMode{ AlphaMode::Opaque };

        float alphaCutoff{ defaults::MaterialAlphaCutoff };
        bool doubleSided{ defaults::MaterialDoubleSided };
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
        int32_t skin{};

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
                    buffer.data.resize(buffer.byteLength);

                    std::ifstream fileData(detail::CreateBufferUriPath(bufferRootPath, buffer.uri), std::ios::binary);
                    if (!fileData.good())
                    {
                        throw invalid_gltf_document("Invalid buffer.uri value", buffer.uri);
                    }

                    fileData.read(&buffer.data[0], buffer.byteLength);
                }
            }

            return document;
        }
    };

    void from_json(nlohmann::json const & json, Accessor::Type & accessorType)
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

    void from_json(nlohmann::json const & json, Accessor & accessor)
    {
        detail::ReadRequiredField("count", json, accessor.count);
        detail::ReadRequiredField("componentType", json, accessor.componentType);
        detail::ReadRequiredField("type", json, accessor.type);

        detail::ReadOptionalField("bufferView", json, accessor.bufferView);
        detail::ReadOptionalField("byteOffset", json, accessor.byteOffset);
        detail::ReadOptionalField("max", json, accessor.max);
        detail::ReadOptionalField("min", json, accessor.min);
        detail::ReadOptionalField("normalized", json, accessor.normalized);
    }

    void from_json(nlohmann::json const & json, Animation::Channel::Target & animationChannelTarget)
    {
        detail::ReadRequiredField("path", json, animationChannelTarget.path);

        detail::ReadOptionalField("node", json, animationChannelTarget.node);
    }

    void from_json(nlohmann::json const & json, Animation::Channel & animationChannel)
    {
        detail::ReadRequiredField("sampler", json, animationChannel.sampler);
        detail::ReadRequiredField("target", json, animationChannel.target);
    }

    void from_json(nlohmann::json const & json, Animation::Sampler::Type & animationSamplerType)
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

    void from_json(nlohmann::json const & json, Animation::Sampler & animationSampler)
    {
        detail::ReadRequiredField("input", json, animationSampler.input);
        detail::ReadRequiredField("ouput", json, animationSampler.ouput);

        detail::ReadOptionalField("interpolation", json, animationSampler.interpolation);
    }

    void from_json(nlohmann::json const & json, Animation & animation)
    {
        detail::ReadRequiredField("channels", json, animation.channels);
        detail::ReadRequiredField("samplers", json, animation.samplers);

        detail::ReadOptionalField("name", json, animation.name);
    }

    void from_json(nlohmann::json const & json, Asset & asset)
    {
        detail::ReadRequiredField("version", json, asset.version);
        detail::ReadOptionalField("copyright", json, asset.copyright);
        detail::ReadOptionalField("generator", json, asset.generator);
        detail::ReadOptionalField("minVersion", json, asset.minVersion);
    }

    void from_json(nlohmann::json const & json, Buffer & buffer)
    {
        detail::ReadRequiredField("byteLength", json, buffer.byteLength);

        detail::ReadOptionalField("name", json, buffer.name);
        detail::ReadOptionalField("uri", json, buffer.uri);
    }

    void from_json(nlohmann::json const & json, BufferView & bufferView)
    {
        detail::ReadRequiredField("buffer", json, bufferView.buffer);
        detail::ReadRequiredField("byteLength", json, bufferView.byteLength);

        detail::ReadOptionalField("name", json, bufferView.name);
        detail::ReadOptionalField("byteOffset", json, bufferView.byteOffset);
        detail::ReadOptionalField("byteStride", json, bufferView.byteStride);
        detail::ReadOptionalField("target", json, bufferView.target);
    }

    void from_json(nlohmann::json const & json, Camera::Orthographic & camera)
    {
        detail::ReadRequiredField("xmag", json, camera.xmag);
        detail::ReadRequiredField("ymag", json, camera.ymag);
        detail::ReadRequiredField("zfar", json, camera.zfar);
        detail::ReadRequiredField("znear", json, camera.znear);
    }

    void from_json(nlohmann::json const & json, Camera::Perspective & camera)
    {
        detail::ReadRequiredField("yfov", json, camera.yfov);
        detail::ReadRequiredField("znear", json, camera.znear);

        detail::ReadOptionalField("aspectRatio", json, camera.aspectRatio);
        detail::ReadOptionalField("zfar", json, camera.zfar);
    }

    void from_json(nlohmann::json const & json, Camera & camera)
    {
        detail::ReadRequiredField("type", json, camera.type);

        detail::ReadOptionalField("name", json, camera.name);
        if (camera.type == "perspective")
        {
            detail::ReadRequiredField("perspective", json, camera.perspective);
        }
        else if (camera.type == "orthographic")
        {
            detail::ReadRequiredField("orthographic", json, camera.orthographic);
        }
        else
        {
            throw invalid_gltf_document("Unknown camera.type value", camera.name);
        }
    }

    void from_json(nlohmann::json const & json, Image & image)
    {
        detail::ReadOptionalField("name", json, image.name);
        detail::ReadOptionalField("uri", json, image.uri);
        detail::ReadOptionalField("mimeType", json, image.mimeType);
        detail::ReadOptionalField("bufferView", json, image.bufferView);
    }

    void from_json(nlohmann::json const & json, Material::AlphaMode & materialAlphaMode)
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

    void from_json(nlohmann::json const & json, Material::Texture & materialTexture)
    {
        detail::ReadRequiredField("index", json, materialTexture.index);
        detail::ReadOptionalField("texCoord", json, materialTexture.texCoord);
    }

    void from_json(nlohmann::json const & json, Material::NormalTexture & materialTexture)
    {
        from_json(json, static_cast<Material::Texture &>(materialTexture));
        detail::ReadOptionalField("scale", json, materialTexture.scale);
    }

    void from_json(nlohmann::json const & json, Material::OcclusionTexture & materialTexture)
    {
        from_json(json, static_cast<Material::Texture &>(materialTexture));
        detail::ReadOptionalField("strength", json, materialTexture.strength);
    }

    void from_json(nlohmann::json const & json, Material::PBRMetallicRoughness & pbrMetallicRoughness)
    {
        detail::ReadOptionalField("baseColorFactor", json, pbrMetallicRoughness.baseColorFactor);
        detail::ReadOptionalField("baseColorTexture", json, pbrMetallicRoughness.baseColorTexture);
        detail::ReadOptionalField("metallicFactor", json, pbrMetallicRoughness.metallicFactor);
        detail::ReadOptionalField("roughnessFactor", json, pbrMetallicRoughness.roughnessFactor);
        detail::ReadOptionalField("metallicRoughnessTexture", json, pbrMetallicRoughness.metallicRoughnessTexture);
    }

    void from_json(nlohmann::json const & json, Material & material)
    {
        detail::ReadOptionalField("name", json, material.name);
        detail::ReadOptionalField("alphaMode", json, material.alphaMode);
        detail::ReadOptionalField("alphaCutoff", json, material.alphaCutoff);
        detail::ReadOptionalField("doubleSided", json, material.doubleSided);
        detail::ReadOptionalField("emissiveFactor", json, material.emissiveFactor);
        detail::ReadOptionalField("emissiveTexture", json, material.emissiveTexture);
        detail::ReadOptionalField("normalTexture", json, material.normalTexture);
        detail::ReadOptionalField("occlusionTexture", json, material.occlusionTexture);
        detail::ReadOptionalField("pbrMetallicRoughness", json, material.pbrMetallicRoughness);
    }

    void from_json(nlohmann::json const & json, Mesh & mesh)
    {
        detail::ReadRequiredField("primitives", json, mesh.primitives);

        detail::ReadOptionalField("name", json, mesh.name);
        detail::ReadOptionalField("weights", json, mesh.weights);
    }

    void from_json(nlohmann::json const & json, Node & node)
    {
        detail::ReadOptionalField("name", json, node.name);
        detail::ReadOptionalField("camera", json, node.camera);
        detail::ReadOptionalField("children", json, node.children);
        detail::ReadOptionalField("skin", json, node.skin);
        detail::ReadOptionalField("mesh", json, node.mesh);
        detail::ReadOptionalField("matrix", json, node.matrix);
        detail::ReadOptionalField("translation", json, node.translation);
        detail::ReadOptionalField("rotation", json, node.rotation);
        detail::ReadOptionalField("scale", json, node.scale);
    }

    void from_json(nlohmann::json const & json, Primitive & primitive)
    {
        detail::ReadRequiredField("attributes", json, primitive.attributes);

        detail::ReadOptionalField("indices", json, primitive.indices);
        detail::ReadOptionalField("material", json, primitive.material);
        detail::ReadOptionalField("mode", json, primitive.mode);
        detail::ReadOptionalField("targets", json, primitive.targets);
    }

    void from_json(nlohmann::json const & json, Sampler & sampler)
    {
        detail::ReadOptionalField("name", json, sampler.name);
        detail::ReadOptionalField("magFilter", json, sampler.magFilter);
        detail::ReadOptionalField("minFilter", json, sampler.minFilter);
        detail::ReadOptionalField("wrapS", json, sampler.wrapS);
        detail::ReadOptionalField("wrapT", json, sampler.wrapT);
    }

    void from_json(nlohmann::json const & json, Scene & scene)
    {
        detail::ReadOptionalField("name", json, scene.name);
        detail::ReadOptionalField("nodes", json, scene.nodes);
    }

    void from_json(nlohmann::json const & json, Skin & skin)
    {
        detail::ReadRequiredField("joints", json, skin.joints);

        detail::ReadOptionalField("name", json, skin.name);
        detail::ReadOptionalField("inverseBindMatrices", json, skin.inverseBindMatrices);
        detail::ReadOptionalField("skeleton", json, skin.skeleton);
    }

    void from_json(nlohmann::json const & json, Texture & texture)
    {
        detail::ReadOptionalField("name", json, texture.name);
        detail::ReadOptionalField("sampler", json, texture.sampler);
        detail::ReadOptionalField("source", json, texture.source);
    }

    void from_json(nlohmann::json const & json, Document & document)
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

    void to_json(nlohmann::json & json, Accessor::ComponentType const & accessorComponentType)
    {
        if (accessorComponentType == Accessor::ComponentType::None)
        {
            throw invalid_gltf_document("Unknown accessor.componentType value");
        }

        json = static_cast<uint16_t>(accessorComponentType);
    }

    void to_json(nlohmann::json & json, Accessor::Type const & accessorType)
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

    void to_json(nlohmann::json & json, Accessor const & accessor)
    {
        detail::WriteField("name", json, accessor.name);
        detail::WriteField("bufferView", json, accessor.bufferView, -1);
        detail::WriteField("byteOffset", json, accessor.byteOffset, {});
        detail::WriteField("count", json, accessor.count, {});
        detail::WriteField("type", json, accessor.type, Accessor::Type::None);
        detail::WriteField("componentType", json, accessor.componentType, Accessor::ComponentType::None);
        detail::WriteField("min", json, accessor.min);
        detail::WriteField("max", json, accessor.max);
        detail::WriteField("normalized", json, accessor.normalized, false);
    }

    void to_json(nlohmann::json & json, Animation::Channel::Target const & animationChannelTarget)
    {
        detail::WriteField("node", json, animationChannelTarget.node, -1);
        detail::WriteField("path", json, animationChannelTarget.path);
    }

    void to_json(nlohmann::json & json, Animation::Channel const & animationChannel)
    {
        detail::WriteField("sampler", json, animationChannel.sampler, -1);
        detail::WriteField("target", json, animationChannel.target);
    }

    void to_json(nlohmann::json & json, Animation::Sampler::Type const & animationSamplerType)
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

    void to_json(nlohmann::json & json, Animation::Sampler const & animationSampler)
    {
        detail::WriteField("input", json, animationSampler.input, -1);
        detail::WriteField("ouput", json, animationSampler.ouput, -1);
        detail::WriteField("interpolation", json, animationSampler.interpolation, Animation::Sampler::Type::Linear);
    }

    void to_json(nlohmann::json & json, Animation const & animation)
    {
        detail::WriteField("channels", json, animation.channels);
        detail::WriteField("samplers", json, animation.samplers);
        detail::WriteField("name", json, animation.name);
    }

    void to_json(nlohmann::json & json, Asset const & asset)
    {
        detail::WriteField("copyright", json, asset.copyright);
        detail::WriteField("generator", json, asset.generator);
        detail::WriteField("minVersion", json, asset.minVersion);
        detail::WriteField("version", json, asset.version);
    }

    void to_json(nlohmann::json & json, Buffer const & buffer)
    {
        detail::WriteField("byteLength", json, buffer.byteLength, {});
        //detail::WriteOptionalField("data", json, buffer.data);
        detail::WriteField("name", json, buffer.name);
        detail::WriteField("uri", json, buffer.uri);
    }

    void to_json(nlohmann::json & json, BufferView const & bufferView)
    {
        detail::WriteField("buffer", json, bufferView.buffer, -1);
        detail::WriteField("byteLength", json, bufferView.byteLength, {});
        detail::WriteField("byteOffset", json, bufferView.byteOffset, static_cast<uint32_t>(-1));
        detail::WriteField("byteStride", json, bufferView.byteStride, {});
        detail::WriteField("name", json, bufferView.name);
        detail::WriteField("target", json, bufferView.target, BufferView::TargetType::None);
    }

    void to_json(nlohmann::json & json, Camera::Orthographic const & camera)
    {
        detail::WriteField("xmag", json, camera.xmag, defaults::FloatSentinal);
        detail::WriteField("ymag", json, camera.ymag, defaults::FloatSentinal);
        detail::WriteField("zfar", json, camera.zfar, defaults::FloatSentinal);
        detail::WriteField("znear", json, camera.znear, defaults::FloatSentinal);
    }

    void to_json(nlohmann::json & json, Camera::Perspective const & camera)
    {
        detail::WriteField("aspectRatio", json, camera.aspectRatio, {});
        detail::WriteField("yfov", json, camera.yfov, {});
        detail::WriteField("zfar", json, camera.zfar, {});
        detail::WriteField("znear", json, camera.znear, {});
    }

    void to_json(nlohmann::json & json, Camera const & camera)
    {
        detail::WriteField("name", json, camera.name);
        detail::WriteField("type", json, camera.type);

        if (camera.type == "perspective")
        {
            detail::WriteField("perspective", json, camera.perspective);
        }
        else if (camera.type == "orthographic")
        {
            detail::WriteField("orthographic", json, camera.orthographic);
        }
        else
        {
            throw invalid_gltf_document("Unknown camera.type value", camera.name);
        }
    }

    void to_json(nlohmann::json & json, Image const & image)
    {
        detail::WriteField("bufferView", json, image.bufferView, {});
        detail::WriteField("mimeType", json, image.mimeType);
        detail::WriteField("name", json, image.name);
        detail::WriteField("uri", json, image.uri);
    }

    void to_json(nlohmann::json & json, Material::AlphaMode const & materialAlphaMode)
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

    void to_json(nlohmann::json & json, Material::Texture const & materialTexture)
    {
        detail::WriteField("index", json, materialTexture.index, -1);
        detail::WriteField("texCoord", json, materialTexture.texCoord, -1);
    }

    void to_json(nlohmann::json & json, Material::NormalTexture const & materialTexture)
    {
        to_json(json, static_cast<Material::Texture const &>(materialTexture));
        detail::WriteField("scale", json, materialTexture.scale, defaults::IdentityScalar);
    }

    void to_json(nlohmann::json & json, Material::OcclusionTexture const & materialTexture)
    {
        to_json(json, static_cast<Material::Texture const &>(materialTexture));
        detail::WriteField("strength", json, materialTexture.strength, defaults::IdentityScalar);
    }

    void to_json(nlohmann::json & json, Material::PBRMetallicRoughness const & pbrMetallicRoughness)
    {
        detail::WriteField("baseColorFactor", json, pbrMetallicRoughness.baseColorFactor, defaults::IdentityVec4);
        detail::WriteField("baseColorTexture", json, pbrMetallicRoughness.baseColorTexture);
        detail::WriteField("metallicFactor", json, pbrMetallicRoughness.metallicFactor, defaults::IdentityScalar);
        detail::WriteField("roughnessFactor", json, pbrMetallicRoughness.roughnessFactor, defaults::IdentityScalar);
        detail::WriteField("metallicRoughnessTexture", json, pbrMetallicRoughness.metallicRoughnessTexture);
    }

    void to_json(nlohmann::json & json, Material const & material)
    {
        detail::WriteField("name", json, material.name);
        detail::WriteField("pbrMetallicRoughness", json, material.pbrMetallicRoughness);
        detail::WriteField("normalTexture", json, material.normalTexture);
        detail::WriteField("occlusionTexture", json, material.occlusionTexture);
        detail::WriteField("emissiveTexture", json, material.emissiveTexture);
        detail::WriteField("emissiveFactor", json, material.emissiveFactor, defaults::NullVec3);
        detail::WriteField("alphaMode", json, material.alphaMode, Material::AlphaMode::Opaque);
        detail::WriteField("alphaCutoff", json, material.alphaCutoff, defaults::MaterialAlphaCutoff);
        detail::WriteField("doubleSided", json, material.doubleSided, defaults::MaterialDoubleSided);
    }

    void to_json(nlohmann::json & json, Mesh const & mesh)
    {
        detail::WriteField("name", json, mesh.name);
        detail::WriteField("weights", json, mesh.weights);
        detail::WriteField("primitives", json, mesh.primitives);
    }

    void to_json(nlohmann::json & json, Node const & node)
    {
        detail::WriteField("name", json, node.name);
        detail::WriteField("camera", json, node.camera, -1);
        detail::WriteField("mesh", json, node.mesh, -1);
        detail::WriteField("skin", json, node.skin, {});
        detail::WriteField("matrix", json, node.matrix, defaults::IdentityMatrix);
        detail::WriteField("rotation", json, node.rotation, defaults::IdentityRotation);
        detail::WriteField("scale", json, node.scale, defaults::IdentityVec3);
        detail::WriteField("translation", json, node.translation, defaults::NullVec3);
        detail::WriteField("children", json, node.children);
        detail::WriteField("weights", json, node.weights);
    }

    void to_json(nlohmann::json & json, Primitive const & primitive)
    {
        detail::WriteField("indices", json, primitive.indices, -1);
        detail::WriteField("material", json, primitive.material, -1);
        detail::WriteField("mode", json, primitive.mode, Primitive::Mode::Triangles);
        detail::WriteField("attributes", json, primitive.attributes);
        detail::WriteField("targets", json, primitive.targets);
    }

    void to_json(nlohmann::json & json, Sampler const & sampler)
    {
        detail::WriteField("name", json, sampler.name);
        detail::WriteField("magFilter", json, sampler.magFilter, Sampler::MagFilter::None);
        detail::WriteField("minFilter", json, sampler.minFilter, Sampler::MinFilter::None);
        detail::WriteField("wrapS", json, sampler.wrapS, Sampler::WrappingMode::Repeat);
        detail::WriteField("wrapT", json, sampler.wrapT, Sampler::WrappingMode::Repeat);
    }

    void to_json(nlohmann::json & json, Scene const & scene)
    {
        detail::WriteField("name", json, scene.name);
        detail::WriteField("nodes", json, scene.nodes);
    }

    void to_json(nlohmann::json & json, Skin const & skin)
    {
        detail::WriteField("inverseBindMatrices", json, skin.inverseBindMatrices, -1);
        detail::WriteField("skeleton", json, skin.skeleton, -1);
        detail::WriteField("joints", json, skin.joints);
        detail::WriteField("name", json, skin.name);
    }

    void to_json(nlohmann::json & json, Texture const & texture)
    {
        detail::WriteField("name", json, texture.name);
        detail::WriteField("sampler", json, texture.sampler, -1);
        detail::WriteField("source", json, texture.source, -1);
    }

    void to_json(nlohmann::json & json, Document const & document)
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

    Document LoadFromText(std::string const & documentFilePath)
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
} // namespace gltf
} // namespace fx

#undef FX_GLTF_HAS_CPP_17
