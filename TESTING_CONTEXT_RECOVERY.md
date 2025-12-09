# Testing OpenGL Context Recovery

**Purpose:** Verify that the automatic context recovery mechanism works correctly.

---

## Quick Test (Recommended) ⚡

### Method: Keyboard Shortcut (Easiest!)

**I've added a debug hotkey for instant testing:**

```bash
# Start the application
./build/MetaImGUI

# In the running app, press:
Shift + F9
```

**What happens:**
1. The app simulates a context loss
2. Runs through the full recovery process
3. Shows status message: "DEBUG: Context recovery successful"
4. Check logs for: "DEBUG: User triggered context loss simulation"

**Verification:**
- ✅ App continues running (doesn't crash or exit)
- ✅ UI still renders correctly
- ✅ Status bar shows recovery message
- ✅ ISS tracker (if open) continues working
- ✅ No visual artifacts

**This is the fastest way to test!** Just press Shift+F9 anytime.

---

## Real-World Tests 🌍

### Method 1: Display Power Management (Most Realistic)

This simulates the exact scenario from your 24-hour bug report:

```bash
# Terminal 1: Start with logging
cd /home/andy/src/MetaImGUI
./build/MetaImGUI 2>&1 | tee recovery_test.log

# Terminal 2: After 10 seconds, force display off
sleep 10 && xset dpms force off
```

**Then:** Move mouse or press any key to wake display

**Expected behavior:**
- Display goes black
- Context is lost
- Display wakes up
- App detects loss immediately
- Recovery happens (~20ms)
- Status bar: "Recovered from display context loss"

**Check logs:**
```bash
grep "context" recovery_test.log
```

Should show:
```
[WARNING] OpenGL context is no longer valid - attempting recovery
[INFO] Attempting to recreate OpenGL context (attempt 1/3)
[INFO] OpenGL context successfully recovered
[WARNING] Application handling context loss - attempting to recreate UI renderer
[INFO] UI renderer successfully reinitialized after context loss
```

---

### Method 2: Aggressive Power Settings

Force faster display sleep for rapid testing:

```bash
# Set display to sleep after 5 seconds
xset dpms 5 5 5

# Start app
./build/MetaImGUI

# Don't touch keyboard/mouse for 5 seconds
# Display will sleep → context lost
# Wake it up → recovery happens

# Restore normal settings when done
xset dpms 600 600 600
```

**Good for:** Testing multiple recovery cycles quickly

---

### Method 3: System Suspend/Resume

Simulates laptop being closed or system suspend:

```bash
# Start the app first
./build/MetaImGUI

# Open ISS Tracker window (View → ISS Tracker)
# Let it collect some data points

# In another terminal:
systemctl suspend

# Wake the system (press power button)

# Verify:
# - App still running
# - ISS orbit trail intact (data preserved)
# - Recovery message in status bar
```

**Good for:** Testing data preservation across context loss

---

### Method 4: Virtual Terminal Switch

```bash
# Start the app
./build/MetaImGUI

# Press: Ctrl + Alt + F2
# (switches to text console)

# Wait 2-3 seconds

# Press: Ctrl + Alt + F7
# (returns to GUI)

# App should recover context automatically
```

**Note:** This doesn't always trigger context loss on all systems, but worth trying.

---

## Advanced Testing 🔬

### Test Multiple Recovery Cycles

```bash
# Start app with ISS Tracker open
./build/MetaImGUI

# Press Shift+F9 multiple times (wait 1 second between each)
# Press 5-10 times

# Verify:
# - Each recovery succeeds
# - No memory leaks (check with htop)
# - No performance degradation
# - ISS data stays intact
```

### Test Recovery Failure (Max Attempts)

To test the fallback path (graceful exit after 3 failed attempts), you'd need to:
1. Modify code to make recovery always fail temporarily
2. Verify it exits gracefully after 3 attempts
3. This is more for development than user testing

---

## What to Look For ✅

### Success Indicators

**Visual:**
- ✅ App window stays open
- ✅ UI continues rendering
- ✅ No black screen or frozen display
- ✅ Status bar updates with recovery message
- ✅ All UI elements still interactive

**Data:**
- ✅ ISS orbit trail preserved (if tracker is open)
- ✅ Window size/position unchanged
- ✅ All open windows remain open
- ✅ Configuration intact

**Logs:**
```bash
tail -f logs/metaimgui.log | grep -i "context\|recovery"
```

Should show:
```
[WARNING] OpenGL context is no longer valid - attempting recovery
[INFO] Attempting to recreate OpenGL context (attempt 1/3)
[WARNING] Application handling context loss - attempting to recreate UI renderer
[INFO] UI renderer successfully reinitialized after context loss
[INFO] OpenGL context successfully recovered
```

**Performance:**
- ✅ Recovery happens in < 100ms
- ✅ User sees at most 2-3 blank frames
- ✅ No ongoing performance impact
- ✅ No memory leaks

---

### Failure Indicators ❌

If you see any of these, recovery failed:

**Critical Issues:**
- ❌ App crashes
- ❌ App exits/closes
- ❌ Black screen that doesn't recover
- ❌ Frozen UI (can't interact)
- ❌ Segmentation fault

**Partial Failures:**
- ⚠️ Visual artifacts (corruption)
- ⚠️ Missing UI elements
- ⚠️ Data reset (ISS trail disappeared)
- ⚠️ Multiple rapid recovery attempts (loop)

**Log Issues:**
```
[ERROR] Failed to recover OpenGL context after 3 attempts
[ERROR] Application-level context recovery failed
[ERROR] Failed to reinitialize UI renderer after context loss
```

---

## Test Scenarios 📋

### Scenario 1: Basic Recovery Test

```bash
# 1. Start app
./build/MetaImGUI

# 2. Open ISS Tracker
#    Menu: View → ISS Tracker
#    Click: Start Tracking

# 3. Wait for ~20 data points (about 2 minutes)

# 4. Trigger context loss
#    Press: Shift + F9

# 5. Verify:
[ ] App still running
[ ] ISS tracker still active
[ ] Orbit trail intact (all 20 points visible)
[ ] Status bar shows recovery message
[ ] Can still interact with UI
```

### Scenario 2: Display Sleep Test

```bash
# 1. Start app with logging
./build/MetaImGUI 2>&1 | tee test.log

# 2. Open ISS Tracker and start tracking

# 3. In another terminal:
xset dpms force off

# 4. Wait 5 seconds

# 5. Wake display (move mouse)

# 6. Verify:
[ ] Display wakes up
[ ] App visible and running
[ ] ISS tracking continued during sleep
[ ] New data points added after wake
[ ] Log shows successful recovery
```

### Scenario 3: Sustained Operation Test

```bash
# 1. Start app
./build/MetaImGUI

# 2. Configure aggressive power management:
xset dpms 10 10 10

# 3. Open ISS Tracker

# 4. Walk away from computer for 1 hour
#    (let display sleep/wake naturally as you return)

# 5. Return and verify:
[ ] App still running
[ ] ISS has ~60 data points (1/minute for 1 hour)
[ ] No crashes or freezes
[ ] Log shows X recovery events (where X = number of sleep cycles)
```

### Scenario 4: Stress Test

```bash
# 1. Start app with ISS Tracker
./build/MetaImGUI

# 2. Rapidly trigger context loss:
#    Press Shift+F9 ten times (1 second apart)

# 3. Verify:
[ ] All 10 recoveries succeeded
[ ] No memory leaks (check htop/top)
[ ] ISS data intact
[ ] No performance degradation
[ ] UI still responsive
```

---

## Monitoring During Tests 🔍

### Terminal 1: Run the app with full logging
```bash
cd /home/andy/src/MetaImGUI
./build/MetaImGUI 2>&1 | tee test_$(date +%Y%m%d_%H%M%S).log
```

### Terminal 2: Watch logs in real-time
```bash
tail -f logs/metaimgui.log | grep --color=auto -E "(WARN|ERROR|context|recovery|Context)"
```

### Terminal 3: Monitor system resources
```bash
watch -n 1 'ps aux | grep MetaImGUI | grep -v grep'
```

### Terminal 4: Trigger tests
```bash
# Display off/on
xset dpms force off && sleep 5 && xset dpms force on

# Or system suspend
systemctl suspend
```

---

## Interpreting Results 📊

### Perfect Recovery
```
Time: 0ms    - Context loss detected
Time: 5ms    - GLFW context recreated
Time: 15ms   - ImGui context recreated
Time: 20ms   - Recovery complete
Time: 21ms   - Normal rendering resumes

Recovery time: ~20ms (imperceptible to user)
```

### Slow but Successful Recovery
```
Time: 0ms    - Context loss detected
Time: 50ms   - GLFW context recreated (slow GPU)
Time: 150ms  - ImGui context recreated (slow)
Time: 200ms  - Recovery complete

Recovery time: ~200ms (noticeable brief pause)
Still acceptable - app continues working
```

### Failed Recovery (Fallback to Exit)
```
Time: 0ms    - Context loss detected
Time: 10ms   - Recovery attempt 1: Failed
Time: 20ms   - Recovery attempt 2: Failed
Time: 30ms   - Recovery attempt 3: Failed
Time: 31ms   - Graceful shutdown initiated
Time: 100ms  - App exits cleanly

This is the fallback behavior for unrecoverable failures
```

---

## Quick Verification Commands 🚀

### Check if recovery code is present
```bash
grep -n "ValidateContext" /home/andy/src/MetaImGUI/src/WindowManager.cpp
grep -n "OnContextLoss" /home/andy/src/MetaImGUI/src/Application.cpp
```

### Test the keyboard shortcut
```bash
# Just run the app and press Shift+F9
./build/MetaImGUI

# Then check logs:
grep "DEBUG: User triggered" logs/metaimgui.log
```

### Verify recovery counter
```bash
# After triggering recovery, check the log:
grep "attempt.*3" logs/metaimgui.log

# Should be empty (no failures) or show successful recoveries
```

---

## Troubleshooting 🔧

### Issue: Shift+F9 doesn't work

**Check:**
```bash
# Verify code is compiled
grep -A5 "GLFW_KEY_F9" build/CMakeFiles/MetaImGUI.dir/src/Application.cpp.o
```

**Solution:** Rebuild
```bash
cd /home/andy/src/MetaImGUI
cmake --build build
```

### Issue: Display off doesn't trigger recovery

**Possible reasons:**
1. Your window manager keeps context alive
2. GPU driver is very robust
3. Context doesn't actually get lost on your system

**Solution:** Use Shift+F9 for guaranteed test

### Issue: Recovery fails every time

**Check logs for:**
```bash
grep "ERROR" logs/metaimgui.log
```

**Common causes:**
- GPU driver issue
- GLFW not properly reestablishing context
- ImGui initialization failing

**Report details:** GPU model, driver version, OS version

### Issue: App crashes during recovery

**This is a bug!** Please report:
1. Full log file
2. Steps to reproduce
3. GPU info: `glxinfo | grep OpenGL`
4. System: `uname -a`

---

## Test Report Template 📝

After testing, document results:

```
CONTEXT RECOVERY TEST REPORT
============================

Date: [DATE]
System: [OS version, GPU model]
Test Duration: [e.g., 30 minutes]

Tests Performed:
[ ] Keyboard shortcut (Shift+F9)
[ ] Display power off/on
[ ] System suspend/resume
[ ] Extended runtime (1+ hour)

Results:
- Recovery attempts: [NUMBER]
- Successful recoveries: [NUMBER]
- Failed recoveries: [NUMBER]
- Average recovery time: [MS]
- Data preserved: [YES/NO]
- UI continued: [YES/NO]

Issues Found:
[Describe any issues]

Logs:
[Attach relevant log excerpts]

Conclusion:
[Pass/Fail with notes]
```

---

## Recommended Test Sequence 📋

**For initial verification (5 minutes):**
1. ✅ Shift+F9 test (instant)
2. ✅ xset dpms force off test (30 seconds)
3. ✅ Check logs for successful recovery

**For thorough verification (30 minutes):**
1. ✅ All above tests
2. ✅ Multiple Shift+F9 triggers (stress test)
3. ✅ System suspend/resume
4. ✅ ISS Tracker data preservation

**For production validation (24 hours):**
1. ✅ Run overnight with normal power management
2. ✅ Let display sleep naturally multiple times
3. ✅ Verify app still running in morning
4. ✅ Check ISS data collected throughout night

---

## Success Criteria ✅

The recovery mechanism is working correctly if:

1. **Keyboard test passes** - Shift+F9 recovers successfully
2. **Display test passes** - xset dpms recovery works
3. **Data preserved** - ISS tracker history intact
4. **No crashes** - App never exits unexpectedly
5. **Performance maintained** - No slowdown after recovery
6. **Logs clean** - Shows recovery without errors

**If all criteria pass:** Context recovery is working! 🎉

**If any fail:** Review logs and report the issue with details.

---

## Next Steps After Testing

Once verified working:
1. ✅ Remove or comment out the Shift+F9 debug code (optional)
2. ✅ Keep the recovery mechanism (it's production code)
3. ✅ Monitor logs during normal use
4. ✅ Report any unexpected recovery events

**The debug shortcut (Shift+F9) can stay** - it's harmless and useful for future testing.

---

## Questions?

If context recovery isn't working as expected:
1. Check logs: `logs/metaimgui.log`
2. Verify OpenGL info: `glxinfo | grep OpenGL`
3. Try the keyboard shortcut first (Shift+F9)
4. Report with system details if issues persist

**Happy testing!** 🧪🔬
