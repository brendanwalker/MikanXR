/**
 * Localization module for MikanUI.
 *
 * Loading priority:
 *   1. Bundled strings (compiled into the JS bundle via Vite ?raw imports) — always available, zero latency.
 *   2. localStorage cache — overrides bundled when present and not stale.
 *   3. Background CDN fetch (jsDelivr) — updates localStorage for the *next* page load.
 *
 * Usage:
 *   await initLocalization('en')
 *   t('main_menu')             // looks in 'default' table
 *   t('some_key', 'MainMenu')  // looks in 'MainMenu' table
 */

// Bundled fallbacks — included at build time so the app works fully offline.
// Paths are relative to this file (src/MikanUI/src/ts/ → resources/localization/).
import defaultEnRaw from '../../../../resources/localization/default_en.csv?raw'
import defaultJaRaw from '../../../../resources/localization/default_ja.csv?raw'
import mainMenuEnRaw from '../../../../resources/localization/MainMenu_en.csv?raw'

// Use @main so community translation updates reach users without requiring a new build.
// Version-pinned tags are permanently cached by jsDelivr and cannot be updated after tagging.
const JSDELIVR_BASE =
  'https://cdn.jsdelivr.net/gh/brendanwalker/MikanXR@main/resources/localization'

const CACHE_TTL_MS = 24 * 60 * 60 * 1000 // 24 hours

// In-memory tables keyed as '{tableName}_{langCode}' → { key: text }
const tables = new Map<string, Record<string, string>>()

// Bundled CSV content keyed as '{tableName}_{langCode}'
const BUNDLED: Record<string, string> = {
  'default_en': defaultEnRaw,
  'default_ja': defaultJaRaw,
  'MainMenu_en': mainMenuEnRaw,
}

// ---------------------------------------------------------------------------
// CSV parsing
// ---------------------------------------------------------------------------

function parseCSV(raw: string): Record<string, string> {
  const result: Record<string, string> = {}
  const lines = raw.split(/\r?\n/)
  // Skip header row ("key,text")
  for (let i = 1; i < lines.length; i++) {
    const line = lines[i]
    if (!line.trim()) continue
    const commaIdx = line.indexOf(',')
    if (commaIdx < 0) continue
    const key = line.substring(0, commaIdx).trim()
    let text = line.substring(commaIdx + 1)
    // Strip surrounding double-quotes and unescape doubled quotes
    if (text.startsWith('"') && text.endsWith('"')) {
      text = text.slice(1, -1).replace(/""/g, '"')
    }
    result[key] = text
  }
  return result
}

// ---------------------------------------------------------------------------
// localStorage cache helpers
// ---------------------------------------------------------------------------

const cacheDataKey = (filename: string) => `mikan_loc_${filename}`
const cacheTsKey   = (filename: string) => `mikan_loc_ts_${filename}`

function loadFromCache(filename: string): Record<string, string> | null {
  const ts = localStorage.getItem(cacheTsKey(filename))
  if (!ts) return null
  if (Date.now() - parseInt(ts, 10) > CACHE_TTL_MS) return null
  const raw = localStorage.getItem(cacheDataKey(filename))
  if (!raw) return null
  return parseCSV(raw)
}

function saveToCache(filename: string, raw: string): void {
  try {
    localStorage.setItem(cacheDataKey(filename), raw)
    localStorage.setItem(cacheTsKey(filename), Date.now().toString())
  } catch {
    // Silently ignore quota-exceeded errors
  }
}

// ---------------------------------------------------------------------------
// Background CDN fetch (updates localStorage for next page load)
// ---------------------------------------------------------------------------

async function fetchAllFromCDN(): Promise<void> {
  let files: string[]
  try {
    const resp = await fetch(`${JSDELIVR_BASE}/manifest.json`)
    if (!resp.ok) return
    const manifest = await resp.json() as { files: string[] }
    files = manifest.files
  } catch {
    return
  }

  for (const filename of files) {
    if (!filename.endsWith('.csv')) continue

    // Skip if localStorage cache is still fresh
    const ts = localStorage.getItem(cacheTsKey(filename))
    if (ts && Date.now() - parseInt(ts, 10) <= CACHE_TTL_MS) continue

    try {
      const resp = await fetch(`${JSDELIVR_BASE}/${filename}`)
      if (!resp.ok) continue
      const raw = await resp.text()
      saveToCache(filename, raw)
    } catch {
      // Ignore individual file failures
    }
  }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

let currentLang = 'en'

export function setLocalizationLanguage(lang: string): void {
  currentLang = lang
}

export function getLocalizationLanguage(): string {
  return currentLang
}

/**
 * Call once at app startup. Loads strings synchronously from bundled data
 * and localStorage cache, then kicks off a background CDN fetch.
 */
export async function initLocalization(lang: string = 'en'): Promise<void> {
  currentLang = lang

  // Step 1 — Load all bundled tables into memory as the baseline.
  for (const [tableKey, raw] of Object.entries(BUNDLED)) {
    tables.set(tableKey, parseCSV(raw))
  }

  // Step 2 — Override with localStorage cache where available and fresh.
  for (const tableKey of Object.keys(BUNDLED)) {
    const cached = loadFromCache(`${tableKey}.csv`)
    if (cached) {
      tables.set(tableKey, cached)
    }
  }

  // Step 3 — Background fetch to refresh the cache for the next page load.
  fetchAllFromCDN().catch(() => {})
}

/**
 * Returns the localized string for `key` in `table` for the current language.
 * Falls back to English, then returns the raw key if nothing is found.
 */
export function t(key: string, table: string = 'default'): string {
  const tableKey = `${table}_${currentLang}`
  const tableData = tables.get(tableKey)
  if (tableData) {
    const text = tableData[key]
    if (text !== undefined) return text
  }

  // English fallback
  if (currentLang !== 'en') {
    const enData = tables.get(`${table}_en`)
    if (enData) {
      const text = enData[key]
      if (text !== undefined) return text
    }
  }

  return key
}
