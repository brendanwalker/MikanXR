#pragma once

// Validates the automation server's pure text-protocol helpers
// (AutomationProtocol::tokenizeCommandLine / frameReply):
//   - whitespace splitting, quoted spans, and escapes tokenize correctly
//   - unterminated quotes report a parse error
//   - replies frame as a count line plus exactly that many content lines
//   - entries with embedded newlines split so the count stays accurate
bool run_automation_protocol_tests();
