# DeepSeek configuration

The project never contains a shared DeepSeek API key. Each developer or player
provides their own key locally in one of these ways:

1. Open **System → AI Settings** in the desktop client. The value is stored by
   Qt `QSettings` for the current OS user and is not written into this repository.
2. Set the `DEEPSEEK_API_KEY` environment variable before starting the client.
   This takes priority over the value in `QSettings` and works well for local
   development or CI.

Do not put a real key in source files, `database.ini`, screenshots, issues, or
commits. If a key is ever pasted into a public place, revoke it in the provider
console and create a replacement.

Local settings protect a key from accidental Git commits, but no desktop client
can make a bundled shared secret impossible to extract. If this project later
ships with a project-funded key, keep that key on a server and let the client call
a narrow authenticated proxy endpoint instead.

The Diagon Alley wand ceremony works without a key: it automatically uses the
bundled wand archive when DeepSeek is unavailable, so the story never blocks.
