# ``CosmoEditor/CosmoEditor``

## Usage

```swift
import CosmoEditor

public struct CodeEditor: View
{
  @State var text: String = "let x = 42"
  @State var cursorPositions: [CursorPosition] = []

  public var body: some View
  {
    CosmoEditor(
      $text,
      language: .default,
      theme: .standard,
      font: .monospacedSystemFont(ofSize: 11, weight: .bold),
      tabWidth: 2,
      indentOption: .spaces(count: 2),
      lineHeight: 1.2,
      wrapLines: true,
      editorOverscroll: 0.3,
      cursorPositions: $cursorPositions
    )
  }
}
```
