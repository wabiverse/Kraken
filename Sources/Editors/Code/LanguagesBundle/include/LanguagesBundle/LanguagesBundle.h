#ifndef __CODE_LANGUAGES_BUNDLE_H__
#define __CODE_LANGUAGES_BUNDLE_H__

typedef struct TSLanguage TSLanguage;

#ifdef __cplusplus
extern "C" {
#endif

// A collection of pointers to supported tree-sitter languages
// Add new ones below (please keep an alphabetical order)

extern TSLanguage *tree_sitter_c();
extern TSLanguage *tree_sitter_cpp();
extern TSLanguage *tree_sitter_galah();
extern TSLanguage *tree_sitter_jsdoc();
extern TSLanguage *tree_sitter_json();
extern TSLanguage *tree_sitter_python();
extern TSLanguage *tree_sitter_rust();
extern TSLanguage *tree_sitter_swift();
extern TSLanguage *tree_sitter_toml();
extern TSLanguage *tree_sitter_usd();

#ifdef __cplusplus
}
#endif

#endif // __CODE_LANGUAGES_BUNDLE_H__
