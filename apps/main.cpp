#include "apps_container.h"
#include "global_preferences.h"
#include <poincare/init.h>

#define DUMMY_MAIN 0
#if DUMMY_MAIN

void ion_main(int argc, const char * const argv[]) {
  // Initialize the backlight
  while (1) {
    Ion::Display::pushRectUniform(KDRect(0,0,320,240), KDColorRed);
    Ion::Timing::msleep(100);
    if (Ion::Keyboard::scan().keyDown(Ion::Keyboard::Key::OnOff)) {
      Ion::Power::suspend();
    }
    Ion::Display::pushRectUniform(KDRect(0,0,320,240), KDColorBlue);
    Ion::Timing::msleep(100);
  }
}

#else

void ion_main(int argc, const char * const argv[]) {
  // Initialize Poincare::TreePool::sharedPool
  Poincare::Init();

#if EPSILON_GETOPT
  for (int i=1; i<argc; i++) {
    if (argv[i][0] != '-' || argv[i][1] != '-') {
      continue;
    }
    /* Option should be given at run-time:
     * $ ./epsilon.elf --language fr
     */
    if (strcmp(argv[i], "--language") == 0 && argc > i+1) {
      const char * requestedLanguageId = argv[i+1];
      if (strcmp(requestedLanguageId, "none") == 0) {
        continue;
      }
      for (int j = 0; j < I18n::NumberOfLanguages; j++) {
        if (strcmp(requestedLanguageId, I18n::LanguageISO6391Codes[j]) == 0) {
          GlobalPreferences::sharedGlobalPreferences()->setLanguage((I18n::Language)j);
          GlobalPreferences::sharedGlobalPreferences()->setCountry(I18n::DefaultCountryForLanguage[j]);
          break;
        }
      }
      continue;
    }
    /* Option should be given at run-time:
     * $ ./epsilon.elf --[app_name]-[option] [arguments]
     * For example:
     * $ make -j8 PLATFORM=emscripten EPSILON_APPS=code
     * $ ./epsilon.elf --code-script hello_world.py:print("hello") --code-lock-on-console
     */
    const char * appNames[] = {"home", EPSILON_APPS_NAMES};
    for (int j = 0; j < AppsContainer::sharedAppsContainer()->numberOfBuiltinApps(); j++) {
      Escher::App::Snapshot * snapshot = AppsContainer::sharedAppsContainer()->appSnapshotAtIndex(j);
      int cmp = strcmp(argv[i]+2, appNames[j]);
      if (cmp == '-') {
        snapshot->setOpt(argv[i]+2+strlen(appNames[j])+1, argv[i+1]);
        break;
      }
    }
  }
#endif

  /* s_stackStart must be defined as early as possible to ensure that there
   * cannot be allocated memory pointers before. Otherwise, with MicroPython for
   * example, stack pointer could go backward after initialization and allocated
   * memory pointers could be overlooked during mark procedure. */
  volatile int stackTop;
  Ion::setStackStart((void *)(&stackTop));

  AppsContainer::sharedAppsContainer()->run();
}

#endif
