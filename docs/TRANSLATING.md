# Translating MikanXR

Every translation in MikanXR started out machine generated. If you speak one of the supported languages, correcting those strings is one of the easiest ways to help, and you do not need to build anything or write any code.

Translations are contributed as pull requests. Once merged, they reach people who already have MikanXR installed within a day, without them downloading a new build.

---

## What you edit

One file: `localization/ja.po` (or whichever language you are working on). These are standard gettext catalogs.

**Do not edit anything under `resources/localization/`.** Those JSON files are generated from the catalogs, and a pull request that edits them by hand will fail its checks.

---

## Getting set up

1. Install [POEdit](https://poedit.net/). It is free, runs on Windows, macOS and Linux, and is what most of this ecosystem uses. Any gettext editor works, including Lokalize and Gtranslator.
2. Fork the repository and clone your fork.
3. Open `localization/<language>.po` in POEdit.

---

## Working through the file

POEdit shows the English text on the left and the translation on the right. Strings marked **"Needs work"** (orange) are the ones to look at: that flag means the string is machine generated, or the English text changed after it was translated.

When you are happy with a translation, clear the "Needs work" flag. That flag is the only record of whether a person has vouched for a string, and it is what the app uses to tell users how much of their language has been reviewed. A string with the flag cleared counts as human reviewed.

If the English source changed under an existing translation, POEdit shows you the previous English text so you can see what moved.

Save the file, commit it, and open a pull request. That is the whole contribution.

---

## Three rules

**Keep the `%s` and `%d` placeholders.** They are where the app inserts a number or a name at runtime. They must appear in the same order and the same kinds as the English text. POEdit will warn you if they do not match. `%%` means a literal percent sign and stays as `%%`.

**Never write `##` in a translation.** Two hash characters in a row are how the UI toolkit separates a label from its internal identifier, so a translation containing them breaks the widget.

**A `\n` is a line break.** Leave it where it makes sense in your language.

Anything else is yours to phrase naturally. Short button labels should stay short, since the UI has fixed-width buttons in places, but there is no hard character limit.

---

## Checking your work before you push

Optional, but it catches mistakes faster than waiting on CI:

```bash
pip install polib
python tools/localization.py check
```

It prints the key of anything wrong. To see how far along each language is:

```bash
python tools/localization.py stats
```

---

## Adding a language that does not exist yet

Open an issue first. A new language usually needs a font that covers its script bundled into the app, which is a code change rather than a translation, and filling in a catalog whose characters cannot render yet is wasted effort. Say which language you want and it can be set up for you.

---

## Questions

Open an issue. Partial translations are welcome: anything you leave untranslated falls back to English, so there is no need to finish a language in one pass.
