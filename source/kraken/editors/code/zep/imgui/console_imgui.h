#pragma once

#include "zep.h"

namespace ANCHOR
{

struct ZepConsole : Zep::IZepComponent
{
  std::function<bool(const std::string &)> Callback;
  Zep::ZepEditor_ANCHOR zepEditor;
  bool pendingScroll = true;

  // Intercept messages from the editor command line and relay them
  virtual void Notify(std::shared_ptr<Zep::ZepMessage> message)
  {
    if (message->messageId == Zep::Msg::HandleCommand)
    {
      message->handled = Callback(message->str);
      return;
    }
    message->handled = false;
    return;
  }

  virtual Zep::ZepEditor &GetEditor() const
  {
    return (Zep::ZepEditor &)zepEditor;
  }

  ZepConsole(Zep::ZepPath &p)
    : zepEditor(p, Zep::NVec2f(1, 1))
  {
    zepEditor.RegisterCallback(this);
    auto pBuffer = zepEditor.GetEmptyBuffer("Log");
    pBuffer->SetFileFlags(Zep::FileFlags::ReadOnly);
  }

  void AddLog(const char *fmt, ...) ANCHOR_FMTARGS(2)
  {
    // FIXME-OPT
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, ANCHOR_ARRAYSIZE(buf), fmt, args);
    buf[ANCHOR_ARRAYSIZE(buf) - 1] = 0;
    va_end(args);

    auto pBuffer = zepEditor.GetMRUBuffer();

    Zep::ChangeRecord record;

    pBuffer->Insert(pBuffer->End(), buf, record);
    pBuffer->Insert(pBuffer->End(), "\n", record);

    pendingScroll = true;
  }

  void Draw(const char *title, bool *p_open, const wabi::GfVec4f &targetRect, float blend)
  {
    ANCHOR::PushStyleColor(ANCHOR_Col_WindowBg, wabi::GfVec4f(0.13f, 0.1f, 0.12f, 0.95f));
    ANCHOR::PushStyleVar(AnchorStyleVar_WindowRounding, 0.0f);
    ANCHOR::SetNextWindowSize(wabi::GfVec2f(targetRect[2], targetRect[3]), ANCHOR_Cond_Always);
    ANCHOR::SetNextWindowPos(
      wabi::GfVec2f(targetRect[0], (targetRect[1] - targetRect[3]) + (targetRect[3] * blend)),
      ANCHOR_Cond_Always);

    if (!ANCHOR::Begin(title,
                       p_open,
                       AnchorWindowFlags_NoTitleBar | AnchorWindowFlags_NoResize |
                         AnchorWindowFlags_NoScrollbar))
    {
      ANCHOR::PopStyleVar(1);
      ANCHOR::PopStyleColor(1);
      ANCHOR::End();
      return;
    }

    auto size = ANCHOR::GetWindowContentRegionMax();
    auto cursor = ANCHOR::GetCursorScreenPos();

    zepEditor.SetDisplayRegion(Zep::NVec2f(cursor[0], cursor[1]), Zep::NVec2f(size[0], size[1] - cursor[1]));
    zepEditor.Display();
    zepEditor.HandleInput();

    if (pendingScroll)
    {
      zepEditor.GetActiveTabWindow()->GetActiveWindow()->MoveCursorY(
        zepEditor.GetActiveTabWindow()->GetActiveWindow()->GetMaxDisplayLines() - 2);
      pendingScroll = false;
    }

    if (blend < 1.0f)
    {
      // TODO: This looks like a hack: investigate why it is needed for the drop down console.
      // I think the intention here is to ensure the mode is reset while it is dropping down. I
      // don't recall.
      zepEditor.GetActiveTabWindow()->GetActiveWindow()->GetBuffer().GetMode()->Begin(
        zepEditor.GetActiveTabWindow()->GetActiveWindow());
    }

    ANCHOR::End();
    ANCHOR::PopStyleColor(1);
    ANCHOR::PopStyleVar(1);
  }
};

}  // namespace ANCHOR
