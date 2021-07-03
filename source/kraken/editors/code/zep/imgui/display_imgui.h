#pragma once
#include "display.h"
#include "syntax.h"

#include "ANCHOR_api.h"

#include <string>

// Can't include this publicly
//#include "zep/mcommon/logger.h"

namespace Zep
{

inline NVec2f toNVec2f(const wabi::GfVec2f &im)
{
  return NVec2f(im[0], im[1]);
}
inline wabi::GfVec2f toNVec2f(const NVec2f &im)
{
  return wabi::GfVec2f(im.x, im.y);
}

static AnchorWChar greek_range[] = {0x300, 0x52F, 0x1f00, 0x1fff, 0, 0};

class ZepFont_ANCHOR : public ZepFont
{
 public:
  ZepFont_ANCHOR(ZepDisplay &display, AnchorFont *pFont, int pixelHeight)
    : ZepFont(display),
      m_pFont(pFont)
  {
    SetPixelHeight(pixelHeight);
  }

  virtual void SetPixelHeight(int pixelHeight) override
  {
    InvalidateCharCache();
    m_pixelHeight = pixelHeight;
  }

  virtual NVec2f GetTextSize(const uint8_t *pBegin, const uint8_t *pEnd = nullptr) const override
  {
    // This is the code from ANCHOR internals; we can't call GetTextSize, because it doesn't return
    // the correct 'advance' formula, which we need as we draw one character at a time...
    const float font_size = m_pFont->FontSize;
    wabi::GfVec2f text_size = m_pFont->CalcTextSizeA(
      float(GetPixelHeight()), FLT_MAX, FLT_MAX, (const char *)pBegin, (const char *)pEnd, NULL);
    if (text_size[0] == 0.0)
    {
      // Make invalid characters a default fixed_size
      const char chDefault = 'A';
      text_size = m_pFont->CalcTextSizeA(
        float(GetPixelHeight()), FLT_MAX, FLT_MAX, &chDefault, (&chDefault + 1), NULL);
    }

    return toNVec2f(text_size);
  }

  AnchorFont *GetAnchorFont()
  {
    return m_pFont;
  }

 private:
  AnchorFont *m_pFont;
  float m_fontScale = 1.0f;
};

class ZepDisplay_ANCHOR : public ZepDisplay
{
 public:
  ZepDisplay_ANCHOR(const NVec2f &pixelScale)
    : ZepDisplay(pixelScale)
  {}

  void DrawChars(ZepFont &font,
                 const NVec2f &pos,
                 const NVec4f &col,
                 const uint8_t *text_begin,
                 const uint8_t *text_end) const override
  {
    auto imFont = static_cast<ZepFont_ANCHOR &>(font).GetAnchorFont();
    ImDrawList *drawList = ANCHOR::GetWindowDrawList();
    if (text_end == nullptr)
    {
      text_end = text_begin + strlen((const char *)text_begin);
    }
    if (m_clipRect.Width() == 0)
    {
      drawList->AddText(imFont,
                        float(font.GetPixelHeight()),
                        toNVec2f(pos),
                        ToPackedABGR(col),
                        (const char *)text_begin,
                        (const char *)text_end);
    }
    else
    {
      drawList->PushClipRect(toNVec2f(m_clipRect.topLeftPx), toNVec2f(m_clipRect.bottomRightPx));
      drawList->AddText(imFont,
                        float(font.GetPixelHeight()),
                        toNVec2f(pos),
                        ToPackedABGR(col),
                        (const char *)text_begin,
                        (const char *)text_end);
      drawList->PopClipRect();
    }
  }

  void DrawLine(const NVec2f &start, const NVec2f &end, const NVec4f &color, float width) const override
  {
    ImDrawList *drawList = ANCHOR::GetWindowDrawList();
    // Background rect for numbers
    if (m_clipRect.Width() == 0)
    {
      drawList->AddLine(toNVec2f(start), toNVec2f(end), ToPackedABGR(color), width);
    }
    else
    {
      drawList->PushClipRect(toNVec2f(m_clipRect.topLeftPx), toNVec2f(m_clipRect.bottomRightPx));
      drawList->AddLine(toNVec2f(start), toNVec2f(end), ToPackedABGR(color), width);
      drawList->PopClipRect();
    }
  }

  void DrawRectFilled(const NRectf &rc, const NVec4f &color) const override
  {
    ImDrawList *drawList = ANCHOR::GetWindowDrawList();
    // Background rect for numbers
    if (m_clipRect.Width() == 0)
    {
      drawList->AddRectFilled(toNVec2f(rc.topLeftPx), toNVec2f(rc.bottomRightPx), ToPackedABGR(color));
    }
    else
    {
      drawList->PushClipRect(toNVec2f(m_clipRect.topLeftPx), toNVec2f(m_clipRect.bottomRightPx));
      drawList->AddRectFilled(toNVec2f(rc.topLeftPx), toNVec2f(rc.bottomRightPx), ToPackedABGR(color));
      drawList->PopClipRect();
    }
  }

  virtual void SetClipRect(const NRectf &rc) override
  {
    m_clipRect = rc;
  }

  virtual ZepFont &GetFont(ZepTextType type) override
  {
    if (m_fonts[(int)type] == nullptr)
    {
      m_fonts[(int)type] = std::make_shared<ZepFont_ANCHOR>(
        *this, ANCHOR::GetIO().Fonts[0].Fonts[FONT_DANKMONO], int(16.0f * GetPixelScale().y));
    }
    return *m_fonts[(int)type];
  }

 private:
  NRectf m_clipRect;
};  // namespace Zep

}  // namespace Zep
