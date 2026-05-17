/* ----------------- Includes: */

#ifndef IncludedLightAPIHeader
#define IncludedLightAPIHeader

#include <limits.h>
#ifndef KERNEL
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#endif
#include <stdarg.h>

/* --------- Architecture defines: */

#if defined(__i386__)
#define LT_ARCH_X86_32
#define LT_BITS_32
#elif defined(__x86_64__)
#define LT_ARCH_X86_64
#define LT_BITS_64
#else
#error Architecture is not supported.
#endif

/* --------- C++/C differences: */

#ifdef __cplusplus

#define LT_EXTERN_C extern "C"
#define LT_CONSTRUCTOR(x) x
#define LT_NULL nullptr

/* Scoped defer: http://www.gingerbill.org/article/defer-in-cpp.html */
template <typename F> struct _EsDefer4 { F f; _EsDefer4(F f) : f(f) {} ~_EsDefer4() { f(); } };
template <typename F> _EsDefer4<F> _EsDeferFunction(F f) { return _EsDefer4<F>(f); }
#define EsDEFER_3(x) LT_C_PREPROCESSOR_JOIN(x, __COUNTER__)
#define _EsDefer5(code) auto EsDEFER_3(_defer_) = _EsDeferFunction([&](){code;})
#define EsDefer(code) _EsDefer5(code)

union EsGeneric {
	uintptr_t u;
	intptr_t i;
	void *p;

	inline EsGeneric() = default;

#ifdef LT_BITS_64
	inline EsGeneric(uintptr_t y) { u = y; }
	inline EsGeneric( intptr_t y) { i = y; }
#endif
	inline EsGeneric(unsigned  y) { u = y; }
	inline EsGeneric(     int  y) { i = y; }
	inline EsGeneric(    void *y) { p = y; }

	inline bool operator==(EsGeneric r) const { return r.u == u; }
};

#else

#define LT_EXTERN_C extern
#define LT_CONSTRUCTOR(x)
#define LT_NULL 0

typedef union {
	uintptr_t u;
	intptr_t i;
	void *p;
} EsGeneric;

typedef struct EsElementPublic EsElementPublic;

#endif

/* --------- Macros: */

#ifdef LT_ARCH_X86_64
#define LT_API_BASE ((void **) 0x1000)
#define LT_SHARED_MEMORY_MAXIMUM_SIZE ((size_t) (1024) * 1024 * 1024 * 1024)
#define LT_PAGE_SIZE ((uintptr_t) 4096)
#define LT_PAGE_BITS (12)

typedef struct EsCRTjmp_buf {
	uintptr_t rsp, rbp, rbx, r12, r13, r14, r15, rip;
} EsCRTjmp_buf;
#endif

#ifdef LT_ARCH_X86_32
#define LT_API_BASE ((void **) 0x1000)
#define LT_SHARED_MEMORY_MAXIMUM_SIZE ((size_t) 1024 * 1024 * 1024)
#define LT_PAGE_SIZE ((uintptr_t) 4096)
#define LT_PAGE_BITS (12)

typedef struct EsCRTjmp_buf {
	uintptr_t esp, ebp, ebx, eip, esi, edi;
} EsCRTjmp_buf;
#endif

LT_EXTERN_C int _EsCRTsetjmp(EsCRTjmp_buf *env);
LT_EXTERN_C __attribute__((noreturn)) void _EsCRTlongjmp(EsCRTjmp_buf *env, int val);
#define EsCRTsetjmp(x) _EsCRTsetjmp(&(x))
#define EsCRTlongjmp(x, y) _EsCRTlongjmp(&(x), (y))

#define _LT_C_PREPROCESSOR_JOIN(x, y) x ## y
#define LT_C_PREPROCESSOR_JOIN(x, y) _LT_C_PREPROCESSOR_JOIN(x, y)

#define EsContainerOf(type, member, pointer) ((type *) ((uint8_t *) pointer - offsetof(type, member)))

#define LT_CHECK_ERROR(x) (((intptr_t) (x)) < (LT_SUCCESS))

#define LT_RECT_1(x) ((EsRectangle) { (int32_t) (x), (int32_t) (x), (int32_t) (x), (int32_t) (x) })
#define LT_RECT_1I(x) ((EsRectangle) { (int32_t) (x), (int32_t) -(x), (int32_t) (x), (int32_t) -(x) })
#define LT_RECT_1S(x) ((EsRectangle) { 0, (int32_t) (x), 0, (int32_t) (x) })
#define LT_RECT_2(x, y) ((EsRectangle) { (int32_t) (x), (int32_t) (x), (int32_t) (y), (int32_t) (y) })
#define LT_RECT_2I(x, y) ((EsRectangle) { (int32_t) (x), (int32_t) -(x), (int32_t) (y), (int32_t) -(y) })
#define LT_RECT_2S(x, y) ((EsRectangle) { 0, (int32_t) (x), 0, (int32_t) (y) })
#define LT_RECT_4(x, y, z, w) ((EsRectangle) { (int32_t) (x), (int32_t) (y), (int32_t) (z), (int32_t) (w) })
#define LT_RECT_4PD(x, y, w, h) ((EsRectangle) { (int32_t) (x), (int32_t) ((x) + (w)), (int32_t) (y), (int32_t) ((y) + (h)) })
#define LT_RECT_WIDTH(_r) ((_r).r - (_r).l)
#define LT_RECT_HEIGHT(_r) ((_r).b - (_r).t)
#define LT_RECT_TOTAL_H(_r) ((_r).r + (_r).l)
#define LT_RECT_TOTAL_V(_r) ((_r).b + (_r).t)
#define LT_RECT_SIZE(_r) LT_RECT_WIDTH(_r), LT_RECT_HEIGHT(_r)
#define LT_RECT_TOP_LEFT(_r) (_r).l, (_r).t
#define LT_RECT_BOTTOM_LEFT(_r) (_r).l, (_r).b
#define LT_RECT_BOTTOM_RIGHT(_r) (_r).r, (_r).b
#define LT_RECT_ALL(_r) (_r).l, (_r).r, (_r).t, (_r).b
#define LT_RECT_VALID(_r) ((_r).l < (_r).r && (_r).t < (_r).b) // This handles extreme values correctly!

#define LT_POINT(x, y) ((EsPoint) { (int32_t) (x), (int32_t) (y) })

#define EsKeyboardIsAltHeld() (EsKeyboardGetModifiers() & LT_MODIFIER_ALT)
#define EsKeyboardIsCtrlHeld() (EsKeyboardGetModifiers() & LT_MODIFIER_CTRL)
#define EsKeyboardIsShiftHeld() (EsKeyboardGetModifiers() & LT_MODIFIER_SHIFT)

#define LT_MEMORY_MOVE_BACKWARDS -

#define EsWaitSingle(object) EsWait(&object, 1, LT_WAIT_NO_TIMEOUT)
#define EsObjectUnmap EsMemoryUnreserve
#define EsElementSetEnabled(element, enabled) EsElementSetDisabled(element, !(enabled))
#define EsCommandSetEnabled(command, enabled) EsCommandSetDisabled(command, !(enabled))
#define EsClipboardHasText(clipboard) EsClipboardHasFormat(clipboard, LT_CLIPBOARD_FORMAT_TEXT)

#define LT_THEME_METRICS_GAP_ALL (LT_THEME_METRICS_GAP_MAJOR | LT_THEME_METRICS_GAP_MINOR | LT_THEME_METRICS_GAP_WRAP)

// Some common layouts...
#define LT_CELL_FILL	 (LT_CELL_H_FILL | LT_CELL_V_FILL)
#define LT_CELL_H_FILL	 (LT_CELL_H_PUSH | LT_CELL_H_EXPAND | LT_CELL_H_SHRINK)
#define LT_CELL_V_FILL	 (LT_CELL_V_PUSH | LT_CELL_V_EXPAND | LT_CELL_V_SHRINK)
#define LT_CELL_CENTER	 (LT_CELL_H_CENTER | LT_CELL_V_CENTER)
#define LT_CELL_PUSH	 (LT_CELL_H_PUSH | LT_CELL_V_PUSH)
#define LT_CELL_EXPAND   (LT_CELL_H_EXPAND | LT_CELL_V_EXPAND)
#define LT_CELL_CORNER	 (LT_CELL_H_LEFT | LT_CELL_V_TOP)
#define LT_CELL_SHRINK	 (LT_CELL_H_SHRINK | LT_CELL_V_SHRINK)
#define LT_CELL_H_CENTER (LT_CELL_H_LEFT | LT_CELL_H_RIGHT)
#define LT_CELL_V_CENTER (LT_CELL_V_TOP | LT_CELL_V_BOTTOM)

#define EsLiteral(x) (char *) x, EsCStringLength((char *) x)

#ifndef LT_INSTANCE_TYPE
#define LT_INSTANCE_TYPE struct EsInstance
#else
struct LT_INSTANCE_TYPE;
#endif
#ifdef __cplusplus
#define EsInstanceCreate(_message, ...) (static_cast<LT_INSTANCE_TYPE *>(_EsInstanceCreate(sizeof(LT_INSTANCE_TYPE), _message, __VA_ARGS__)))
#else
#define EsInstanceCreate(_message, ...) ((LT_INSTANCE_TYPE *) _EsInstanceCreate(sizeof(LT_INSTANCE_TYPE), _message, __VA_ARGS__))
#endif

#define LT_SAMPLE_FORMAT_BYTES_PER_SAMPLE(x) \
	((x) == LT_SAMPLE_FORMAT_U8 ? 1 : (x) == LT_SAMPLE_FORMAT_S16LE ? 2 : 4)

#define LT_EXTRACT_BITS(value, end, start) (((value) >> (start)) & ((1 << ((end) - (start) + 1)) - 1))   /* Moves the bits to the start. */
#define LT_ISOLATE_BITS(value, end, start) (((value)) & (((1 << ((end) - (start) + 1)) - 1) << (start))) /* Keeps the bits in place. */

#ifndef KERNEL
#ifdef LT_API
LT_EXTERN_C uintptr_t _APISyscall(uintptr_t argument0, uintptr_t argument1, uintptr_t argument2, uintptr_t unused, uintptr_t argument3, uintptr_t argument4);
#ifdef PAUSE_ON_USERLAND_CRASH
LT_EXTERN_C uintptr_t APISyscallCheckForCrash(uintptr_t argument0, uintptr_t argument1, uintptr_t argument2, uintptr_t unused, uintptr_t argument3, uintptr_t argument4);
#define EsSyscall(a, b, c, d, e) APISyscallCheckForCrash((a), (b), (c), 0, (d), (e))
#define _EsSyscall APISyscallCheckForCrash
#else
#define EsSyscall(a, b, c, d, e) _APISyscall((a), (b), (c), 0, (d), (e))
#define _EsSyscall _APISyscall
#endif
#else
#define EsSyscall(a, b, c, d, e) _EsSyscall((a), (b), (c), 0, (d), (e))
#endif
#endif

#define LT_STRUCT_PACKED __attribute__((packed))
#define LT_FUNCTION_OPTIMISE_O2 __attribute__((optimize("-O2")))
#define LT_FUNCTION_OPTIMISE_O3 __attribute__((optimize("-O3")))

#ifdef LT_BITS_64
#define LT_PTR64_MS32(x) ((uint32_t) ((uintptr_t) (x) >> 32))
#define LT_PTR64_LS32(x) ((uint32_t) ((uintptr_t) (x) & 0xFFFFFFFF))
#else
#define LT_PTR64_MS32(x) ((uint32_t) (0))
#define LT_PTR64_LS32(x) ((uint32_t) (x))
#endif

#define EsPerformanceTimerPush() double _performanceTimerStart = EsTimeStampMs()
#define EsPerformanceTimerPop() ((EsTimeStampMs() - _performanceTimerStart) / 1000.0)

/* --------- Algorithms: */

#define LT_MACRO_SORT(_name, _type, _compar, _contextType) void _name(_type *base, size_t nmemb, _contextType context) { \
	(void) context; \
	if (nmemb <= 1) return; \
	\
	if (nmemb <= 16) { \
		for (uintptr_t i = 1; i < nmemb; i++) { \
			for (intptr_t j = i; j > 0; j--) { \
				_type *_left = base + j, *_right = _left - 1; \
				int result; _compar if (result >= 0) break; \
				\
				_type swap = base[j]; \
				base[j] = base[j - 1]; \
				base[j - 1] = swap; \
			} \
		} \
		\
		return; \
	} \
	\
	intptr_t i = -1, j = nmemb; \
	\
	while (true) { \
		_type *_left, *_right = base; \
		int result; \
		\
		while (true) { _left = base + ++i; _compar if (result >= 0) break; } \
		while (true) { _left = base + --j; _compar if (result <= 0) break; } \
		\
		if (i >= j) break; \
		\
		_type swap = base[i]; \
		base[i] = base[j]; \
		base[j] = swap; \
	} \
	\
	_name(base, ++j, context); \
	_name(base + j, nmemb - j, context); \
} \

#define LT_MACRO_SEARCH(_count, _compar, _result, _found) \
	do { \
		if (_count) { \
			intptr_t low = 0; \
			intptr_t high = _count - 1; \
			\
			while (low <= high) { \
				uintptr_t index = ((high - low) >> 1) + low; \
				int result; \
				_compar \
				\
				if (result < 0) { \
					high = index - 1; \
				} else if (result > 0) { \
					low = index + 1; \
				} else { \
					_result = index; \
					_found = true; \
					break; \
				} \
			} \
			\
			if (high < low) { \
				_result = low; \
				_found = false; \
			} \
		} else { \
			_result = 0; \
			_found = false; \
		} \
	} while (0)

/* --------- Misc: */

typedef uint64_t _EsLongConstant;
typedef long double EsLongDouble;
typedef const char *EsCString;

#ifndef LT_API
LT_EXTERN_C void _init();
LT_EXTERN_C void _start();
#endif

#define EsAssert(x) do { if (!(x)) { EsAssertionFailure(__FILE__, __LINE__); } } while (0)
#define EsCRTassert EsAssert

#define LT_INFINITY __builtin_inff()
#define LT_PI (3.1415926535897932384626433832795028841971693994)

/* --------- Internals: */

#if defined(LT_API) || defined(KERNEL)
#define LT_PRIVATE_APIS
#endif

#ifdef LT_PRIVATE_APIS

struct _EsPOSIXSyscall {
	intptr_t index;
	intptr_t arguments[7];
};

#define BLEND_WINDOW_MATERIAL_NONE       (0)
#define BLEND_WINDOW_MATERIAL_GLASS      (1)
#define BLEND_WINDOW_MATERIAL_LIGHT_BLUR (2)

#ifdef LT_ARCH_X86_64
#define BUNDLE_FILE_MAP_ADDRESS (0x100000000UL)
#define BUNDLE_FILE_DESKTOP_MAP_ADDRESS (0xF0000000UL)
#endif

#ifdef LT_ARCH_X86_32
#define BUNDLE_FILE_MAP_ADDRESS (0xA0000000UL)
#define BUNDLE_FILE_DESKTOP_MAP_ADDRESS (0xBC000000UL)
#endif

struct BundleHeader {
#define BUNDLE_SIGNATURE (0x63BDAF45)
	uint32_t signature;
	uint32_t version;
	uint32_t fileCount;
	uint32_t _unused;
	uint64_t mapAddress;
};

struct BundleFile {
	uint64_t nameCRC64;
	uint64_t bytes;
	uint64_t offset;
};

struct MemoryAvailable {
	size_t available;
	size_t total;
};

struct GlobalData {
	volatile int32_t clickChainTimeoutMs;
	volatile float uiScale;
	volatile bool swapLeftAndRightButtons;
	volatile bool showCursorShadow;
	volatile bool useSmartQuotes;
	volatile bool enableHoverState;
	volatile float animationTimeMultiplier;
	volatile uint64_t schedulerTimeMs;
	volatile uint64_t schedulerTimeOffset;
	volatile uint16_t keyboardLayout;
};

struct SystemStartupDataHeader {
	/* TODO Make mount points and devices equal, somehow? */
	size_t initialMountPointCount;
	size_t initialDeviceCount;
	uintptr_t themeCursorData;
	uintptr_t desktopRequestPipe, desktopResponsePipe;
};

#ifdef KERNEL
#define K_BOOT_DRIVE ""
#else
#define K_BOOT_DRIVE "0:"
#endif

#define K_SYSTEM_FOLDER_NAME "Light"
#define K_SYSTEM_FOLDER K_BOOT_DRIVE "/" K_SYSTEM_FOLDER_NAME
#define K_DESKTOP_EXECUTABLE K_SYSTEM_FOLDER "/Desktop.lpp"
#define K_SYSTEM_CONFIGURATION K_SYSTEM_FOLDER "/Default.ini"

#define WINDOW_SET_BITS_NORMAL (0)
#define WINDOW_SET_BITS_AFTER_RESIZE (1)

#define FAST_SCROLL_HORIZONTAL (1)
#define FAST_SCROLL_VERTICAL (2)
#define FAST_SCROLL_DO_NOT_ATTEMPT (3)

#define CURSOR_USE_ACCELERATION (1 << 0)
#define CURSOR_USE_ALT_SLOW (1 << 1)
#define CURSOR_SPEED(x) ((x) >> 16)
#define CURSOR_TRAILS(x) (((x) >> 13) & 7)

#ifdef __cplusplus
extern "C" const void *EsBufferRead(struct EsBuffer *buffer, size_t readBytes);
extern "C" const void *EsBufferReadMany(struct EsBuffer *buffer, size_t a, size_t b);
extern "C" void *EsBufferWrite(EsBuffer *buffer, const void *source, size_t writeBytes);
#define EsBuffer_MEMBER_FUNCTIONS \
	inline const void *Read(size_t readBytes) { return EsBufferRead(this, readBytes); } \
	inline const void *Read(size_t a, size_t b) { return EsBufferReadMany(this, a, b); } \
	inline void *Write(const void *source, size_t writeBytes) { return EsBufferWrite(this, source, writeBytes); }
#endif

#define LT_POSIX_SYSCALL_GET_POSIX_FD_PATH (0x10000)

#define DESKTOP_MESSAGE_SIZE_LIMIT (0x4000)

#define LT_THEME_CURSORS_WIDTH (264)
#define LT_THEME_CURSORS_HEIGHT (128)

/* Messages sent from Kernel to Desktop: */
#define LT_MSG_EMBEDDED_WINDOW_DESTROYED 	((EsMessageType) (LT_MSG_SYSTEM_START + 0x001))
#define LT_MSG_APPLICATION_CRASH		((EsMessageType) (LT_MSG_SYSTEM_START + 0x002))

/* Messages sent from Desktop to application instances: */
#define LT_MSG_TAB_INSPECT_UI			((EsMessageType) (LT_MSG_SYSTEM_START + 0x101))
#define LT_MSG_TAB_CLOSE_REQUEST		((EsMessageType) (LT_MSG_SYSTEM_START + 0x102))
#define LT_MSG_INSTANCE_SAVE_RESPONSE		((EsMessageType) (LT_MSG_SYSTEM_START + 0x103)) /* Sent by Desktop after an application requested to save its document. */
#define LT_MSG_INSTANCE_DOCUMENT_RENAMED	((EsMessageType) (LT_MSG_SYSTEM_START + 0x104))
#define LT_MSG_INSTANCE_DOCUMENT_UPDATED	((EsMessageType) (LT_MSG_SYSTEM_START + 0x105))
#define LT_MSG_INSTANCE_RENAME_RESPONSE		((EsMessageType) (LT_MSG_SYSTEM_START + 0x107))

/* Misc messages: */
#define LT_MSG_EYEDROP_REPORT			((EsMessageType) (LT_MSG_SYSTEM_START + 0x201))
#define LT_MSG_TIMER				((EsMessageType) (LT_MSG_SYSTEM_START + 0x202))
#define LT_MSG_PING				((EsMessageType) (LT_MSG_SYSTEM_START + 0x203)) /* Sent by Desktop to check processes are processing messages. */
#define LT_MSG_WAKEUP				((EsMessageType) (LT_MSG_SYSTEM_START + 0x204)) /* Sent to wakeup the message thread, so that it can process locally posted messages. */
#define LT_MSG_INSTANCE_OPEN_DELAYED		((EsMessageType) (LT_MSG_SYSTEM_START + 0x205))
#define LT_MSG_INSTANCE_SAVE_COMPLETE_DELAYED   ((EsMessageType) (LT_MSG_SYSTEM_START + 0x206))

#endif

/* --------- CRT function macros: */

#ifdef LT_CRT_WITHOUT_PREFIX
#define abs EsCRTabs
#define acosf EsCRTacosf
#define asinf EsCRTasinf
#define assert EsCRTassert
#define atan2 EsCRTatan2
#define atan2f EsCRTatan2f
#define atanf EsCRTatanf
#define atod EsCRTatod
#define atof EsCRTatof
#define atoi EsCRTatoi
#define bsearch EsCRTbsearch
#define calloc EsCRTcalloc
#define cbrt EsCRTcbrt
#define cbrtf EsCRTcbrtf
#define ceil EsCRTceil
#define ceilf EsCRTceilf
#define cos EsCRTcos
#define cosf EsCRTcosf
#define exp EsCRTexp
#define expf EsCRTexpf
#define exp2 EsCRTexp2
#define exp2f EsCRTexp2f
#define fabs EsCRTfabs
#define fabsf EsCRTfabsf
#define floor EsCRTfloor
#define floorf EsCRTfloorf
#define fmod EsCRTfmod
#define fmodf EsCRTfmodf
#define free EsCRTfree
#define getenv EsCRTgetenv
#define isalpha EsCRTisalpha
#define isdigit EsCRTisdigit
#define isnanf EsCRTisnanf
#define isspace EsCRTisspace
#define isupper EsCRTisupper
#define isxdigit EsCRTisxdigit
#define malloc EsCRTmalloc
#define memchr EsCRTmemchr
#define memcmp EsCRTmemcmp
#define memcpy EsCRTmemcpy
#define memmove EsCRTmemmove
#define memset EsCRTmemset
#define pow EsCRTpow
#define powf EsCRTpowf
#define qsort EsCRTqsort
#define rand EsCRTrand
#define realloc EsCRTrealloc
#define sin EsCRTsin
#define sinf EsCRTsinf
#define snprintf EsCRTsnprintf
#define sprintf EsCRTsprintf
#define sqrt EsCRTsqrt
#define sqrtf EsCRTsqrtf
#define strcat EsCRTstrcat
#define strchr EsCRTstrchr
#define strcmp EsCRTstrcmp
#define strcpy EsCRTstrcpy
#define strdup EsCRTstrdup
#define strerror EsCRTstrerror
#define strlen EsCRTstrlen
#define strncmp EsCRTstrncmp
#define strncpy EsCRTstrncpy
#define strnlen EsCRTstrnlen
#define strstr EsCRTstrstr
#define strtod EsCRTstrtod
#define strtof EsCRTstrtof
#define strtol EsCRTstrtol
#define strtoul EsCRTstrtoul
#define tolower EsCRTtolower
#define vsnprintf EsCRTvsnprintf
#endif
