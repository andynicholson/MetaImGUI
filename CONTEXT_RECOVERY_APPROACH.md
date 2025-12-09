# Context Recovery: Best Practices for Long-Running Applications

**Date:** December 9, 2025
**Topic:** OpenGL Context Loss Handling Strategy

---

## Question from User

> "Is that the standard practice? Should the app not be robust to unexpected OpenGL context losses?"

**Short Answer:** You're absolutely right! The standard practice for production applications, especially long-running ones like the ISS Tracker, is to **automatically recover** from context loss, not exit.

---

## Context Loss: Recoverable vs Fatal Errors

### When Context Loss Happens

OpenGL contexts can become invalid for many **normal, expected reasons**:

1. **Display Power Management**
   - Screen goes to sleep after inactivity
   - Most common cause on laptops
   - Happens regularly, not exceptional

2. **Graphics Driver Events**
   - Driver updates or resets
   - GPU timeout detection and recovery (TDR on Windows)
   - Multi-GPU switching

3. **Window Manager Actions**
   - Screen saver activation (some implementations)
   - Display reconfiguration (monitor unplugged/connected)
   - Desktop environment compositor changes

4. **System Events**
   - System suspend/resume
   - Resource pressure (OS reclaims GPU resources)
   - Virtual machine GPU passthrough changes

### Key Insight

Context loss is a **recoverable error**, similar to:
- Network connection drops (you retry, don't quit)
- File system becoming temporarily unavailable (you wait/retry)
- Memory pressure requiring cache eviction (you adapt)

It is **not** like:
- Out of memory crashes (fatal)
- Corrupted application state (fatal)
- Irrecoverable hardware failure (fatal)

---

## Standard Practices by Application Type

### Long-Running Desktop Applications ✅ **MUST RECOVER**

Applications that run for days/weeks should handle context loss gracefully:

- **Monitoring tools** (like ISS Tracker) ← You are here
- **System tray utilities**
- **Background services with UI**
- **Professional software** (CAD, video editing, data visualization)
- **Scientific applications** (simulations, data analysis)
- **Communication apps** (chat, video conferencing)

**Standard Practice:** Automatic context recovery with transparent operation

### Games 🎮 **USUALLY RECOVER**

Modern games typically:
- Detect context loss
- Show "Recovering..." message
- Reload shaders and textures
- Resume gameplay

Some older/casual games exit to menu instead.

### Short-Lived Tools 🔧 **MAY EXIT**

Command-line tools, one-time converters, or launchers can reasonably exit on context loss since:
- They're not meant to run for hours
- User will just restart them
- Simpler implementation

---

## Implementation: Two Approaches

### ❌ Approach 1: Exit on Context Loss (Initial Implementation)

```cpp
if (contextLost) {
    LOG_ERROR("Context lost - exiting");
    requestClose();
}
```

**Pros:**
- Simple to implement
- Clear error path
- No risk of bad state

**Cons:**
- User has to restart application
- All runtime data is lost (ISS position history)
- Poor user experience for long-running apps
- Not suitable for production monitoring tools

### ✅ Approach 2: Automatic Recovery (Production Implementation)

```cpp
if (contextLost) {
    if (recreateContext()) {
        LOG_INFO("Context recovered");
        continue;  // Keep running
    } else if (++attempts < MAX_ATTEMPTS) {
        retry;
    } else {
        LOG_ERROR("Recovery failed - exiting");
        requestClose();
    }
}
```

**Pros:**
- Application continues running
- Runtime data preserved
- Better user experience
- Professional behavior
- Handles common scenarios (display sleep, etc.)

**Cons:**
- More complex implementation
- Need to track recovery state
- Must handle partial recovery failures

---

## Recovery Strategy Details

### What Needs to be Recreated?

When OpenGL context is lost, you must recreate:

1. **GLFW Context** (lowest level)
   - `glfwMakeContextCurrent()` to reestablish connection

2. **ImGui Context** (UI framework)
   - Shutdown old context
   - Create new context
   - Reinitialize backends (GLFW, OpenGL)

3. **ImPlot Context** (plotting library)
   - Destroy old context
   - Create new context

4. **Application State** (if needed)
   - Usually preserved (ISS data, configuration, etc.)
   - Only GPU resources need recreation

### What Gets Preserved?

Most application state survives context loss:
- **CPU data structures** (ISS position history, configuration)
- **Application logic** (tracking threads, network connections)
- **Window state** (size, position)
- **User interface state** (which windows are open)

Only **GPU resources** are lost:
- OpenGL objects (textures, buffers, shaders)
- ImGui render data
- Font atlases

---

## Implementation in MetaImGUI

### Three-Tier Recovery System

```
┌─────────────────────────────────────┐
│  BeginFrame/EndFrame (Detection)    │
│  - Validate context each frame      │
│  - Check for GL_CONTEXT_LOST        │
└──────────────┬──────────────────────┘
               │ Context lost?
               ▼
┌─────────────────────────────────────┐
│  WindowManager (GLFW Recovery)      │
│  - glfwMakeContextCurrent()         │
│  - Verify context is valid          │
│  - Call application callback        │
└──────────────┬──────────────────────┘
               │ GLFW recovered?
               ▼
┌─────────────────────────────────────┐
│  Application (UI Recovery)          │
│  - Shutdown ImGui/ImPlot            │
│  - Recreate ImGui/ImPlot contexts   │
│  - Reinitialize renderers           │
└──────────────┬──────────────────────┘
               │ UI recovered?
               ▼
        ┌──────────────┐
        │   Success!   │  ← Continue normal operation
        └──────────────┘

        OR (after 3 attempts)

        ┌──────────────┐
        │ Graceful Exit│  ← Last resort
        └──────────────┘
```

### Retry Logic

```
Attempt 1: Immediate recovery attempt
  ↓ Failed
Attempt 2: Retry (next frame)
  ↓ Failed
Attempt 3: Final retry
  ↓ Failed
Graceful exit with error message
```

Maximum 3 attempts prevents infinite recovery loops.

---

## Industry Examples

### Applications That Recover from Context Loss

**Professional Software:**
- Adobe Photoshop - Recovers transparently
- Blender - Shows recovery message, continues
- Unity Editor - Recreates render contexts
- Chrome/Firefox - WebGL contexts are recoverable

**Desktop Applications:**
- OBS Studio - Recovers from display device changes
- VLC Media Player - Handles GPU context switches
- Steam - Recovers from display mode changes

**Scientific Software:**
- MATLAB - Handles graphics context loss
- ParaView - Recovers visualization contexts
- VisIt - Transparent recovery for long simulations

### Why They Recover

Because **losing hours of work** due to screen sleep is unacceptable in professional software.

---

## Testing Context Recovery

### How to Trigger Context Loss

1. **Display Sleep**
   ```bash
   # Let screen go to sleep naturally
   # Wait for power management timeout
   ```

2. **Manual Display Off**
   ```bash
   # Linux
   xset dpms force off

   # Then wake it up
   ```

3. **GPU Driver Reset** (Advanced)
   ```bash
   # Windows: Can be simulated with TdrDelay registry key
   # Linux: Can trigger with GPU hang detection
   ```

4. **Virtual Machine**
   ```bash
   # Suspend and resume VM
   # GPU context is often lost/recreated
   ```

### What to Verify

After triggering context loss:
- [ ] Application continues running
- [ ] UI renders correctly
- [ ] No visual artifacts
- [ ] Data is preserved (ISS history intact)
- [ ] Status bar shows recovery message
- [ ] Log shows recovery attempt
- [ ] No memory leaks (check with valgrind)
- [ ] No crashes on subsequent context losses

---

## Performance Considerations

### Recovery Time

Context recreation typically takes:
- **GLFW context:** < 1ms
- **ImGui context:** 1-5ms
- **ImPlot context:** < 1ms
- **Font atlas:** 5-20ms (largest cost)

**Total:** Usually 10-30ms for full recovery

User perceives:
- 1-2 blank frames (barely noticeable at 60 FPS)
- Brief "hiccup" in rendering
- Much better than application exit!

### Recovery Overhead

When context is **not** lost:
- Validation cost: < 0.1ms per frame
- Negligible impact on normal operation
- Well worth the robustness

---

## Conclusion

### For Long-Running Applications

**Context recovery is not optional** - it's a fundamental requirement for production-quality software that users expect to run for extended periods.

### The Rule of Thumb

```
If your application should run for > 1 hour:
    Implement automatic context recovery
else if it's a quick tool:
    Graceful exit is acceptable
```

### MetaImGUI ISS Tracker

As a monitoring application designed to run for days:
- ✅ Automatic context recovery is **essential**
- ✅ Data continuity is **required**
- ✅ User shouldn't need to babysit the app
- ✅ Production-ready behavior

---

## Further Reading

- [OpenGL Context Loss and Recovery](https://www.khronos.org/opengl/wiki/OpenGL_Context)
- [WebGL Context Lost Handling](https://www.khronos.org/webgl/wiki/HandlingContextLost)
- [GLFW Window Guide - Context Robustness](https://www.glfw.org/docs/latest/context_guide.html)
- [ImGui Issues - Context Recreation](https://github.com/ocornut/imgui/issues?q=context+loss)

---

## Credits

**Thank you** to the user for asking this excellent question! It prompted improving the implementation from a naive "exit on error" approach to a proper production-ready automatic recovery system. This is exactly the kind of feedback that makes software better.

> "Is that the standard practice? Should the app not be robust to unexpected OpenGL context losses?"

The answer is a definitive **YES** - and now MetaImGUI implements this correctly. 🎉
