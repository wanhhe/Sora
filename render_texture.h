#pragma once
#include <string>

class FrameBufferTexture
{
public:
    unsigned int color_buffer;
    int width;
    int height;
    std::string name;

    FrameBufferTexture(int _width, int _height);
    virtual ~FrameBufferTexture();
    void SetAsRenderTarget();
    void SetAsReadTarget();
    void BindFrameBuffer();
    static void ClearBufferBinding() { 
        // �޸�
        glBindFramebuffer(GL_FRAMEBUFFER, 0); 
    }

protected:
    virtual void CreateFrameBuffer(int _width, int _height) = 0;
    unsigned int framebuffer = 0;
};

class RenderTexture : public FrameBufferTexture
{
public:
    RenderTexture(int _width, int _height);
    unsigned int GetFrameBuffer() { return framebuffer; }
    unsigned int GetRenderBuffer() { return renderbuffer; }
    virtual ~RenderTexture();

protected:
    void CreateFrameBuffer(int _width, int _height) override;
    unsigned int renderbuffer = 0;
};

class BloomRenderBuffer : public RenderTexture
{
public:
    unsigned int bright_buffer;
    BloomRenderBuffer(int _width, int _height);
    ~BloomRenderBuffer();

private:
    void CreateFrameBuffer(int _width, int _height) override;
};

class DepthTexture : public FrameBufferTexture
{
public:
    DepthTexture(int _width, int _height);
    ~DepthTexture();

private:
    void CreateFrameBuffer(int _width, int _height) override;
};

class SkyboxTexture : public RenderTexture
{
public:
    unsigned int environment_cubemap_buffer;
    SkyboxTexture(int _width, int _height);
    ~SkyboxTexture();

private:
    void CreateFrameBuffer(int _width, int _height) override;
};

class IrradianceTexture
{
public:
    unsigned int irradiance_cubemap_buffer;
    int width;
    int height;
    std::string name;
    IrradianceTexture(int _width, int _height, unsigned int _framebuffer, unsigned int _renderbuffer);
    ~IrradianceTexture();
    unsigned int GetFrameBuffer() { return framebuffer; }
    unsigned int GetRenderBuffer() { return renderbuffer; }

protected:
    unsigned int framebuffer = 0;
    unsigned int renderbuffer = 0;
};

class OtherFrameBufferAndRenderBufferTexture2D
{
public:
    unsigned int color_buffer;
    int width;
    int height;
    std::string name;
    OtherFrameBufferAndRenderBufferTexture2D(int _width, int _height, unsigned int _framebuffer, unsigned int _renderbuffer);
    ~OtherFrameBufferAndRenderBufferTexture2D();
    unsigned int GetFrameBuffer() { return framebuffer; }
    unsigned int GetRenderBuffer() { return renderbuffer; }

protected:
    unsigned int framebuffer = 0;
    unsigned int renderbuffer = 0;
};

class SignleCubeMapTexture
{
public:
    unsigned int cubemap_buffer;
    int width;
    int height;
    std::string name;
    SignleCubeMapTexture(int _width, int _height);
    ~SignleCubeMapTexture();

protected:
};