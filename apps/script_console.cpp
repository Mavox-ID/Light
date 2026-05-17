#define LT_INSTANCE_TYPE Instance
#include <light.h>
#include <shared/array.cpp>

struct LaxValue;
struct LaxEnvironment;

enum LaxType {
    LAX_NIL, LAX_BOOL, LAX_NUMBER, LAX_STRING, LAX_SYMBOL, LAX_PAIR, LAX_PROCEDURE, LAX_ERROR
};

struct LaxValue {
    LaxType type;
    union {
        bool boolVal;
        double numVal;
        struct { char* data; size_t length; } strVal;
        struct { LaxValue* car; LaxValue* cdr; } pairVal;
        struct { LaxValue* (*proc)(LaxValue* args, LaxEnvironment* env); const char* name; } procVal;
    };
    LaxValue() : type(LAX_NIL) {}
};

struct LaxBinding {
    char* name;
    LaxValue* value;
    LaxBinding* next;
};

struct LaxEnvironment {
    LaxBinding* bindings;
    LaxEnvironment* parent;
};

struct Instance : EsInstance {
    EsCommand commandClearOutput;
    EsThreadInformation scriptThread;
    char *inputText;
    size_t inputBytes;
    EsPanel *root;
    EsPanel *inputRow;
    EsPanel *outputPanel;
    EsElement *logOutputGroup;
    uintptr_t logGroupNestLevel;
    EsSpacer *outputDecoration;
    EsTextbox *inputTextbox;
    char *outputLineBuffer;
    size_t outputLineBufferBytes;
    size_t outputLineBufferAllocated;
    bool anyOutput;
    bool gotREPLResult;
    Array<EsElement *> outputElements;
    bool waitingForInput;
    char *inputPrompt;
    char *inputResult;
    bool inputReady;
    int executionCount;
    bool executionLimitReached;
    LaxEnvironment* globalEnv;
};

EsFileInformation globalCommandHistoryFile;
Instance *scriptInstance;

void AddPrompt(Instance *instance);
void AddOutput(Instance *instance, const char *text, size_t textBytes);

#define COLOR_BACKGROUND (0xFFFDFDFD)
#define COLOR_OUTPUT_DECORATION_IN_PROGRLTS (0xFFFF7F00)
#define COLOR_OUTPUT_DECORATION_SUCCLTS (0xFF3070FF)
#define COLOR_OUTPUT_DECORATION_FAILURE (0xFFFF3040)
#define COLOR_TEXT_MAIN (0xFF010102)
#define COLOR_TEXT_LIGHT (0xFF606062)
#define TEXT_SIZE_DEFAULT (13)
#define TEXT_SIZE_OUTPUT (12)

const EsStyle styleBackground = {
    .appearance = { .enabled = true, .backgroundColor = COLOR_BACKGROUND },
};

const EsStyle styleRoot = {
    .metrics = { .mask = LT_THEME_METRICS_INSETS | LT_THEME_METRICS_GAP_ALL,
        .insets = LT_RECT_4(32, 32, 32, 32), .gapMajor = 4 },
};

const EsStyle styleInputRow = {
    .metrics = { .mask = LT_THEME_METRICS_GAP_ALL, .gapMajor = 8 },
};

const EsStyle styleOutputParagraph = {
    .metrics = { .mask = LT_THEME_METRICS_FONT_FAMILY | LT_THEME_METRICS_FONT_WEIGHT
        | LT_THEME_METRICS_TEXT_SIZE | LT_THEME_METRICS_TEXT_ALIGN | LT_THEME_METRICS_TEXT_COLOR,
        .textColor = COLOR_TEXT_MAIN, .textAlign = LT_TEXT_H_LEFT | LT_TEXT_WRAP | LT_TEXT_V_TOP,
        .textSize = TEXT_SIZE_OUTPUT, .fontFamily = LT_FONT_SANS, .fontWeight = 4 },
};

const EsStyle stylePromptText = {
    .metrics = { .mask = LT_THEME_METRICS_FONT_FAMILY | LT_THEME_METRICS_FONT_WEIGHT
        | LT_THEME_METRICS_TEXT_SIZE | LT_THEME_METRICS_TEXT_ALIGN | LT_THEME_METRICS_TEXT_COLOR,
        .textColor = COLOR_TEXT_LIGHT, .textAlign = LT_TEXT_H_LEFT | LT_TEXT_V_CENTER,
        .textSize = TEXT_SIZE_DEFAULT, .fontFamily = LT_FONT_SANS, .fontWeight = 5 },
};

const EsStyle styleInputTextbox = {
    .inherit = LT_STYLE_TEXTBOX_TRANSPARENT,
    .metrics = { .mask = LT_THEME_METRICS_INSETS | LT_THEME_METRICS_FONT_FAMILY | LT_THEME_METRICS_FONT_WEIGHT
        | LT_THEME_METRICS_TEXT_SIZE | LT_THEME_METRICS_TEXT_COLOR,
        .insets = LT_RECT_4(4, 4, 4, 4),
        .textColor = COLOR_TEXT_MAIN, .textSize = TEXT_SIZE_DEFAULT,
        .fontFamily = LT_FONT_SANS, .fontWeight = 4 },
    .appearance = { .enabled = true, .borderColor = 0xFFCCCCCC, .borderSize = LT_RECT_4(1, 1, 1, 1) },
};

const EsStyle styleCommandLogText = {
    .metrics = { .mask = LT_THEME_METRICS_FONT_FAMILY | LT_THEME_METRICS_FONT_WEIGHT
        | LT_THEME_METRICS_TEXT_SIZE | LT_THEME_METRICS_TEXT_ALIGN | LT_THEME_METRICS_TEXT_COLOR,
        .textColor = COLOR_TEXT_MAIN, .textAlign = LT_TEXT_H_LEFT | LT_TEXT_ELLIPSIS | LT_TEXT_V_TOP,
        .textSize = TEXT_SIZE_DEFAULT, .fontFamily = LT_FONT_SANS, .fontWeight = 4 },
};

const EsStyle styleInterCommandSpacer = {
    .metrics = { .mask = LT_THEME_METRICS_PREFERRED_HEIGHT, .preferredHeight = 14 },
};

const EsStyle styleOutputPanelWrapper = {
    .metrics = { .mask = LT_THEME_METRICS_INSETS | LT_THEME_METRICS_GAP_MAJOR,
        .insets = LT_RECT_1(10), .gapMajor = 12 },
};

const EsStyle styleOutputPanel = {
    .metrics = { .mask = LT_THEME_METRICS_INSETS | LT_THEME_METRICS_GAP_MAJOR,
        .insets = LT_RECT_2(0, 4), .gapMajor = 0 },
};

const EsStyle styleOutputDecorationInProgress = {
    .metrics = { .mask = LT_THEME_METRICS_PREFERRED_WIDTH, .preferredWidth = 6 },
    .appearance = { .enabled = true, .backgroundColor = COLOR_OUTPUT_DECORATION_IN_PROGRLTS },
};

const EsStyle styleOutputDecorationSuccess = {
    .metrics = { .mask = LT_THEME_METRICS_PREFERRED_WIDTH, .preferredWidth = 6 },
    .appearance = { .enabled = true, .backgroundColor = COLOR_OUTPUT_DECORATION_SUCCLTS },
};

const EsStyle styleOutputDecorationFailure = {
    .metrics = { .mask = LT_THEME_METRICS_PREFERRED_WIDTH, .preferredWidth = 6 },
    .appearance = { .enabled = true, .backgroundColor = COLOR_OUTPUT_DECORATION_FAILURE },
};

LaxValue* laxAllocValue() {
    return (LaxValue*)EsHeapAllocate(sizeof(LaxValue), true);
}

LaxValue* laxMakeNumber(double n) {
    LaxValue* v = laxAllocValue();
    v->type = LAX_NUMBER;
    v->numVal = n;
    return v;
}

LaxValue* laxMakeString(const char* s) {
    LaxValue* v = laxAllocValue();
    v->type = LAX_STRING;
    size_t len = EsCStringLength(s);
    v->strVal.data = (char*)EsHeapAllocate(len + 1, false);
    EsMemoryCopy(v->strVal.data, s, len + 1);
    v->strVal.length = len;
    return v;
}

LaxValue* laxMakeSymbol(const char* s) {
    LaxValue* v = laxAllocValue();
    v->type = LAX_SYMBOL;
    size_t len = EsCStringLength(s);
    v->strVal.data = (char*)EsHeapAllocate(len + 1, false);
    EsMemoryCopy(v->strVal.data, s, len + 1);
    v->strVal.length = len;
    return v;
}

LaxValue* laxMakeBool(bool b) {
    LaxValue* v = laxAllocValue();
    v->type = LAX_BOOL;
    v->boolVal = b;
    return v;
}

LaxValue* laxMakeNil() {
    LaxValue* v = laxAllocValue();
    v->type = LAX_NIL;
    return v;
}

LaxValue* laxMakePair(LaxValue* car, LaxValue* cdr) {
    LaxValue* v = laxAllocValue();
    v->type = LAX_PAIR;
    v->pairVal.car = car;
    v->pairVal.cdr = cdr;
    return v;
}

LaxValue* laxMakeError(const char* msg) {
    LaxValue* v = laxAllocValue();
    v->type = LAX_ERROR;
    size_t len = EsCStringLength(msg);
    v->strVal.data = (char*)EsHeapAllocate(len + 1, false);
    EsMemoryCopy(v->strVal.data, msg, len + 1);
    v->strVal.length = len;
    return v;
}

LaxValue* laxMakeProcedure(LaxValue* (*proc)(LaxValue* args, LaxEnvironment* env), const char* name) {
    LaxValue* v = laxAllocValue();
    v->type = LAX_PROCEDURE;
    v->procVal.proc = proc;
    v->procVal.name = name;
    return v;
}

LaxEnvironment* laxCreateEnvironment(LaxEnvironment* parent) {
    LaxEnvironment* env = (LaxEnvironment*)EsHeapAllocate(sizeof(LaxEnvironment), true);
    env->bindings = nullptr;
    env->parent = parent;
    return env;
}

void laxDefine(LaxEnvironment* env, const char* name, LaxValue* value) {
    LaxBinding* b = env->bindings;
    while (b) {
        if (EsCRTstrcmp(b->name, name) == 0) {
            b->value = value;
            return;
        }
        b = b->next;
    }
    LaxBinding* binding = (LaxBinding*)EsHeapAllocate(sizeof(LaxBinding), true);
    size_t len = EsCStringLength(name);
    binding->name = (char*)EsHeapAllocate(len + 1, false);
    EsMemoryCopy(binding->name, name, len + 1);
    binding->value = value;
    binding->next = env->bindings;
    env->bindings = binding;
}

LaxValue* laxLookup(LaxEnvironment* env, const char* name) {
    while (env) {
        LaxBinding* b = env->bindings;
        while (b) {
            if (EsCRTstrcmp(b->name, name) == 0) return b->value;
            b = b->next;
        }
        env = env->parent;
    }
    return nullptr;
}

bool laxIsTruthy(LaxValue* v) {
    if (v->type == LAX_BOOL) return v->boolVal;
    if (v->type == LAX_NIL) return false;
    return true;
}

int laxListLength(LaxValue* list) {
    int len = 0;
    while (list->type == LAX_PAIR) {
        len++;
        list = list->pairVal.cdr;
    }
    return len;
}

LaxValue* laxEval(LaxValue* expr, LaxEnvironment* env);

double laxSqrtImpl(double x) {
    if (x < 0) return 0;
    double result = 1.0;
    for (int i = 0; i < 20; i++) {
        result = (result + x / result) / 2.0;
    }
    return result;
}

LaxValue* laxBuiltinSqrt(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) != 1) return laxMakeError("sqrt needs 1 arg");
    LaxValue* val = laxEval(args->pairVal.car, env);
    if (val->type == LAX_ERROR) return val;
    if (val->type != LAX_NUMBER) return laxMakeError("sqrt: not a number");
    if (val->numVal < 0) return laxMakeError("sqrt: negative number");
    return laxMakeNumber(laxSqrtImpl(val->numVal));
}

LaxValue* laxBuiltinSqr(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) != 1) return laxMakeError("sqr needs 1 arg");
    LaxValue* val = laxEval(args->pairVal.car, env);
    if (val->type == LAX_ERROR) return val;
    if (val->type != LAX_NUMBER) return laxMakeError("sqr: not a number");
    return laxMakeNumber(val->numVal * val->numVal);
}

LaxValue* laxBuiltinImage(LaxValue* args, LaxEnvironment* env) {
    if (args->type == LAX_NIL) {
        return laxMakeError("image: No arguments to execute");
    }
    char buffer[4096];
    buffer[0] = '\0';
    while (args->type == LAX_PAIR) {
        LaxValue* val = laxEval(args->pairVal.car, env);
        if (val->type == LAX_ERROR) return val;
        if (val->type == LAX_STRING) {
            EsCRTstrcat(buffer, val->strVal.data);
        } else if (val->type == LAX_NUMBER) {
            char numBuf[64];
            double n = val->numVal;
            // Check for infinity and NaN
            if (n != n) {
                EsCRTstrcat(buffer, "nan");
            } else if (n > 1e308) {
                EsCRTstrcat(buffer, "inf");
            } else if (n < -1e308) {
                EsCRTstrcat(buffer, "-inf");
            } else {
                EsCRTsnprintf(numBuf, sizeof(numBuf), "%g", n);
                EsCRTstrcat(buffer, numBuf);
            }
        } else if (val->type == LAX_BOOL) {
            EsCRTstrcat(buffer, val->boolVal ? "true" : "false");
        } else if (val->type == LAX_NIL) {
            EsCRTstrcat(buffer, "nil");
        } else {
            EsCRTstrcat(buffer, "?");
        }
        args = args->pairVal.cdr;
    }
    AddOutput(scriptInstance, buffer, EsCStringLength(buffer));
    AddOutput(scriptInstance, "\n", 1);
    return laxMakeNil();
}

LaxValue* laxBuiltinInput(LaxValue* args, LaxEnvironment* env) {
    char prompt[256] = "Write your answer: ";
    
    if (args->type == LAX_PAIR) {
        LaxValue* promptVal = laxEval(args->pairVal.car, env);
        if (promptVal->type == LAX_ERROR) return promptVal;
        if (promptVal->type == LAX_STRING) {
            if (promptVal->strVal.length == 0) {
                prompt[0] = '\0';
            } else {
                EsCRTstrncpy(prompt, promptVal->strVal.data, sizeof(prompt) - 1);
                prompt[sizeof(prompt) - 1] = '\0';
            }
        }
    }
    
    if (scriptInstance->outputLineBufferBytes > 0) {
        if (EsUTF8IsValid(scriptInstance->outputLineBuffer, scriptInstance->outputLineBufferBytes)) {
            EsMessageMutexAcquire();
            EsTextDisplayCreate(scriptInstance->outputPanel, LT_CELL_H_FILL | LT_TEXT_DISPLAY_RICH_TEXT,
                    EsStyleIntern(&styleOutputParagraph),
                    scriptInstance->outputLineBuffer, scriptInstance->outputLineBufferBytes);
            EsMessageMutexRelease();
        }
        scriptInstance->outputLineBufferBytes = 0;
    }
    
    if (prompt[0]) {
        EsMessageMutexAcquire();
        EsTextDisplayCreate(scriptInstance->outputPanel, LT_CELL_H_FILL | LT_TEXT_DISPLAY_RICH_TEXT,
                EsStyleIntern(&styleOutputParagraph), prompt, EsCStringLength(prompt));
        EsMessageMutexRelease();
    }
    
    scriptInstance->waitingForInput = true;
    scriptInstance->inputReady = false;
    
    size_t promptLen = EsCStringLength(prompt);
    if (scriptInstance->inputPrompt) {
        EsHeapFree(scriptInstance->inputPrompt);
    }
    scriptInstance->inputPrompt = (char*)EsHeapAllocate(promptLen + 1, false);
    EsMemoryCopy(scriptInstance->inputPrompt, prompt, promptLen + 1);
    
    EsMessageMutexAcquire();
    AddPrompt(scriptInstance);
    EsMessageMutexRelease();
    
    while (!scriptInstance->inputReady) {
        EsSleep(10);
    }
    
    LaxValue* result = laxMakeString(scriptInstance->inputResult);
    EsHeapFree(scriptInstance->inputResult);
    scriptInstance->inputResult = nullptr;
    scriptInstance->waitingForInput = false;
    
    return result;
}

LaxValue* laxBuiltinAbs(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) != 1) return laxMakeError("abs needs 1 arg");
    LaxValue* val = laxEval(args->pairVal.car, env);
    if (val->type == LAX_ERROR) return val;
    if (val->type != LAX_NUMBER) return laxMakeError("abs: not a number");
    return laxMakeNumber(val->numVal < 0 ? -val->numVal : val->numVal);
}

LaxValue* laxBuiltinPow(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) != 2) return laxMakeError("pow needs 2 args");
    LaxValue* base = laxEval(args->pairVal.car, env);
    if (base->type == LAX_ERROR) return base;
    LaxValue* exp = laxEval(args->pairVal.cdr->pairVal.car, env);
    if (exp->type == LAX_ERROR) return exp;
    if (base->type != LAX_NUMBER || exp->type != LAX_NUMBER) return laxMakeError("pow: not numbers");
    double result = 1.0, b = base->numVal;
    int e = (int)exp->numVal;
    bool neg = e < 0; if (neg) e = -e;
    for (int i = 0; i < e; i++) result *= b;
    return laxMakeNumber(neg ? 1.0 / result : result);
}

LaxValue* laxBuiltinMax(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) < 2) return laxMakeError("max needs 2+ args");
    double m = -1e308;
    while (args->type == LAX_PAIR) {
        LaxValue* v = laxEval(args->pairVal.car, env);
        if (v->type == LAX_ERROR) return v;
        if (v->type != LAX_NUMBER) return laxMakeError("max: not a number");
        if (v->numVal > m) m = v->numVal;
        args = args->pairVal.cdr;
    }
    return laxMakeNumber(m);
}

LaxValue* laxBuiltinMin(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) < 2) return laxMakeError("min needs 2+ args");
    double m = 1e308;
    while (args->type == LAX_PAIR) {
        LaxValue* v = laxEval(args->pairVal.car, env);
        if (v->type == LAX_ERROR) return v;
        if (v->type != LAX_NUMBER) return laxMakeError("min: not a number");
        if (v->numVal < m) m = v->numVal;
        args = args->pairVal.cdr;
    }
    return laxMakeNumber(m);
}

LaxValue* laxBuiltinFloor(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) != 1) return laxMakeError("floor needs 1 arg");
    LaxValue* v = laxEval(args->pairVal.car, env);
    if (v->type == LAX_ERROR) return v;
    if (v->type != LAX_NUMBER) return laxMakeError("floor: not a number");
    double n = v->numVal;
    long i = (long)n;
    if (n < 0 && n != (double)i) i--;
    return laxMakeNumber((double)i);
}

LaxValue* laxBuiltinCeil(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) != 1) return laxMakeError("ceil needs 1 arg");
    LaxValue* v = laxEval(args->pairVal.car, env);
    if (v->type == LAX_ERROR) return v;
    if (v->type != LAX_NUMBER) return laxMakeError("ceil: not a number");
    double n = v->numVal;
    long i = (long)n;
    if (n > 0 && n != (double)i) i++;
    return laxMakeNumber((double)i);
}

LaxValue* laxBuiltinRound(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) != 1) return laxMakeError("round needs 1 arg");
    LaxValue* v = laxEval(args->pairVal.car, env);
    if (v->type == LAX_ERROR) return v;
    if (v->type != LAX_NUMBER) return laxMakeError("round: not a number");
    return laxMakeNumber((double)(long)(v->numVal + (v->numVal >= 0 ? 0.5 : -0.5)));
}

LaxValue* laxBuiltinMod(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) != 2) return laxMakeError("mod needs 2 args");
    LaxValue* a = laxEval(args->pairVal.car, env);
    if (a->type == LAX_ERROR) return a;
    LaxValue* b = laxEval(args->pairVal.cdr->pairVal.car, env);
    if (b->type == LAX_ERROR) return b;
    if (a->type != LAX_NUMBER || b->type != LAX_NUMBER) return laxMakeError("mod: not numbers");
    if (b->numVal == 0) return laxMakeError("mod: division by zero");
    long ai = (long)a->numVal, bi = (long)b->numVal;
    return laxMakeNumber((double)(ai % bi));
}

LaxValue* laxBuiltinNot(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) != 1) return laxMakeError("not needs 1 arg");
    LaxValue* v = laxEval(args->pairVal.car, env);
    if (v->type == LAX_ERROR) return v;
    return laxMakeBool(!laxIsTruthy(v));
}

LaxValue* laxBuiltinAnd(LaxValue* args, LaxEnvironment* env) {
    while (args->type == LAX_PAIR) {
        LaxValue* v = laxEval(args->pairVal.car, env);
        if (v->type == LAX_ERROR) return v;
        if (!laxIsTruthy(v)) return laxMakeBool(false);
        args = args->pairVal.cdr;
    }
    return laxMakeBool(true);
}

LaxValue* laxBuiltinOr(LaxValue* args, LaxEnvironment* env) {
    while (args->type == LAX_PAIR) {
        LaxValue* v = laxEval(args->pairVal.car, env);
        if (v->type == LAX_ERROR) return v;
        if (laxIsTruthy(v)) return laxMakeBool(true);
        args = args->pairVal.cdr;
    }
    return laxMakeBool(false);
}

LaxValue* laxBuiltinSin(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) != 1) return laxMakeError("sin needs 1 arg");
    LaxValue* v = laxEval(args->pairVal.car, env);
    if (v->type == LAX_ERROR) return v;
    if (v->type != LAX_NUMBER) return laxMakeError("sin: not a number");
    double x = v->numVal, r = 0, t = x;
    for (int i = 1; i <= 12; i++) { r += t; t *= -x*x/((2*i)*(2*i+1)); }
    return laxMakeNumber(r);
}

LaxValue* laxBuiltinCos(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) != 1) return laxMakeError("cos needs 1 arg");
    LaxValue* v = laxEval(args->pairVal.car, env);
    if (v->type == LAX_ERROR) return v;
    if (v->type != LAX_NUMBER) return laxMakeError("cos: not a number");
    double x = v->numVal, r = 1, t = 1;
    for (int i = 1; i <= 12; i++) { t *= -x*x/((2*i-1)*(2*i)); r += t; }
    return laxMakeNumber(r);
}

LaxValue* laxBuiltinStrlen(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) != 1) return laxMakeError("strlen needs 1 arg");
    LaxValue* v = laxEval(args->pairVal.car, env);
    if (v->type == LAX_ERROR) return v;
    if (v->type != LAX_STRING) return laxMakeError("strlen: not a string");
    return laxMakeNumber((double)v->strVal.length);
}

LaxValue* laxBuiltinStrcat(LaxValue* args, LaxEnvironment* env) {
    char buffer[4096]; buffer[0] = '\0';
    while (args->type == LAX_PAIR) {
        LaxValue* v = laxEval(args->pairVal.car, env);
        if (v->type == LAX_ERROR) return v;
        if (v->type == LAX_STRING) EsCRTstrcat(buffer, v->strVal.data);
        else if (v->type == LAX_NUMBER) {
            char nb[64]; EsCRTsnprintf(nb, sizeof(nb), "%g", v->numVal);
            EsCRTstrcat(buffer, nb);
        } else if (v->type == LAX_BOOL) EsCRTstrcat(buffer, v->boolVal ? "true" : "false");
        args = args->pairVal.cdr;
    }
    return laxMakeString(buffer);
}

LaxValue* laxBuiltinNumToStr(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) != 1) return laxMakeError("numstr needs 1 arg");
    LaxValue* v = laxEval(args->pairVal.car, env);
    if (v->type == LAX_ERROR) return v;
    if (v->type != LAX_NUMBER) return laxMakeError("numstr: not a number");
    char buf[64]; EsCRTsnprintf(buf, sizeof(buf), "%g", v->numVal);
    return laxMakeString(buf);
}

LaxValue* laxBuiltinPrint(LaxValue* args, LaxEnvironment* env) {
    return laxBuiltinImage(args, env);
}

LaxValue* laxBuiltinTan(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) != 1) return laxMakeError("tan needs 1 arg");
    LaxValue* v = laxEval(args->pairVal.car, env); if (v->type == LAX_ERROR) return v;
    if (v->type != LAX_NUMBER) return laxMakeError("tan: not a number");
    double x = v->numVal;
    double s = x, t = x; for (int i=1;i<=12;i++){t *= -x*x/((2*i)*(2*i+1));s+=t;}
    double c = 1, ct = 1; for (int i=1;i<=12;i++){ct *= -x*x/((2*i-1)*(2*i));c+=ct;}
    if (c == 0) return laxMakeError("tan: undefined");
    return laxMakeNumber(s/c);
}

LaxValue* laxBuiltinLog(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) != 1) return laxMakeError("log needs 1 arg");
    LaxValue* v = laxEval(args->pairVal.car, env); if (v->type == LAX_ERROR) return v;
    if (v->type != LAX_NUMBER) return laxMakeError("log: not a number");
    if (v->numVal <= 0) return laxMakeError("log: must be positive");
    double y = (v->numVal - 1.0) / (v->numVal + 1.0), r = 0, t = y;
    for (int i=0;i<30;i++){r+=t/(2*i+1);t*=y*y;}
    return laxMakeNumber(2.0*r);
}

LaxValue* laxBuiltinLog10(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) != 1) return laxMakeError("log10 needs 1 arg");
    LaxValue* v = laxEval(args->pairVal.car, env); if (v->type == LAX_ERROR) return v;
    if (v->type != LAX_NUMBER) return laxMakeError("log10: not a number");
    if (v->numVal <= 0) return laxMakeError("log10: must be positive");
    double y = (v->numVal - 1.0) / (v->numVal + 1.0), r = 0, t = y;
    for (int i=0;i<30;i++){r+=t/(2*i+1);t*=y*y;}
    return laxMakeNumber(2.0*r/2.302585092994046);
}

LaxValue* laxBuiltinExp(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) != 1) return laxMakeError("exp needs 1 arg");
    LaxValue* v = laxEval(args->pairVal.car, env); if (v->type == LAX_ERROR) return v;
    if (v->type != LAX_NUMBER) return laxMakeError("exp: not a number");
    double x=v->numVal, r=1, t=1;
    for (int i=1;i<=20;i++){t*=x/i;r+=t;}
    return laxMakeNumber(r);
}

LaxValue* laxBuiltinNeg(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) != 1) return laxMakeError("neg needs 1 arg");
    LaxValue* v = laxEval(args->pairVal.car, env); if (v->type == LAX_ERROR) return v;
    if (v->type != LAX_NUMBER) return laxMakeError("neg: not a number");
    return laxMakeNumber(-v->numVal);
}

LaxValue* laxBuiltinSign(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) != 1) return laxMakeError("sign needs 1 arg");
    LaxValue* v = laxEval(args->pairVal.car, env); if (v->type == LAX_ERROR) return v;
    if (v->type != LAX_NUMBER) return laxMakeError("sign: not a number");
    return laxMakeNumber(v->numVal > 0 ? 1 : v->numVal < 0 ? -1 : 0);
}

LaxValue* laxBuiltinClamp(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) != 3) return laxMakeError("clamp needs 3 args");
    LaxValue* a = laxEval(args->pairVal.car, env); if (a->type==LAX_ERROR) return a;
    LaxValue* mn = laxEval(args->pairVal.cdr->pairVal.car, env); if (mn->type==LAX_ERROR) return mn;
    LaxValue* mx = laxEval(args->pairVal.cdr->pairVal.cdr->pairVal.car, env); if (mx->type==LAX_ERROR) return mx;
    if (a->type!=LAX_NUMBER||mn->type!=LAX_NUMBER||mx->type!=LAX_NUMBER) return laxMakeError("clamp: not numbers");
    double n=a->numVal; if(n<mn->numVal) n=mn->numVal; if(n>mx->numVal) n=mx->numVal;
    return laxMakeNumber(n);
}

LaxValue* laxBuiltinLerp(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args) != 3) return laxMakeError("lerp needs 3 args");
    LaxValue* a = laxEval(args->pairVal.car, env); if (a->type==LAX_ERROR) return a;
    LaxValue* b = laxEval(args->pairVal.cdr->pairVal.car, env); if (b->type==LAX_ERROR) return b;
    LaxValue* t = laxEval(args->pairVal.cdr->pairVal.cdr->pairVal.car, env); if (t->type==LAX_ERROR) return t;
    if (a->type!=LAX_NUMBER||b->type!=LAX_NUMBER||t->type!=LAX_NUMBER) return laxMakeError("lerp: not numbers");
    return laxMakeNumber(a->numVal+(b->numVal-a->numVal)*t->numVal);
}

LaxValue* laxBuiltinIsNum(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args)!=1) return laxMakeError("isnum needs 1 arg");
    LaxValue* v=laxEval(args->pairVal.car,env); if(v->type==LAX_ERROR)return v;
    return laxMakeBool(v->type==LAX_NUMBER);
}
LaxValue* laxBuiltinIsStr(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args)!=1) return laxMakeError("isstr needs 1 arg");
    LaxValue* v=laxEval(args->pairVal.car,env); if(v->type==LAX_ERROR)return v;
    return laxMakeBool(v->type==LAX_STRING);
}
LaxValue* laxBuiltinIsBool(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args)!=1) return laxMakeError("isbool needs 1 arg");
    LaxValue* v=laxEval(args->pairVal.car,env); if(v->type==LAX_ERROR)return v;
    return laxMakeBool(v->type==LAX_BOOL);
}
LaxValue* laxBuiltinEven(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args)!=1) return laxMakeError("even needs 1 arg");
    LaxValue* v=laxEval(args->pairVal.car,env); if(v->type==LAX_ERROR)return v;
    if(v->type!=LAX_NUMBER) return laxMakeError("even: not a number");
    return laxMakeBool(((long)v->numVal%2)==0);
}
LaxValue* laxBuiltinOdd(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args)!=1) return laxMakeError("odd needs 1 arg");
    LaxValue* v=laxEval(args->pairVal.car,env); if(v->type==LAX_ERROR)return v;
    if(v->type!=LAX_NUMBER) return laxMakeError("odd: not a number");
    return laxMakeBool(((long)v->numVal%2)!=0);
}
LaxValue* laxBuiltinPositive(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args)!=1) return laxMakeError("positive needs 1 arg");
    LaxValue* v=laxEval(args->pairVal.car,env); if(v->type==LAX_ERROR)return v;
    if(v->type!=LAX_NUMBER) return laxMakeError("positive: not a number");
    return laxMakeBool(v->numVal>0);
}
LaxValue* laxBuiltinNegative(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args)!=1) return laxMakeError("negative needs 1 arg");
    LaxValue* v=laxEval(args->pairVal.car,env); if(v->type==LAX_ERROR)return v;
    if(v->type!=LAX_NUMBER) return laxMakeError("negative: not a number");
    return laxMakeBool(v->numVal<0);
}
LaxValue* laxBuiltinZero(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args)!=1) return laxMakeError("zero needs 1 arg");
    LaxValue* v=laxEval(args->pairVal.car,env); if(v->type==LAX_ERROR)return v;
    if(v->type!=LAX_NUMBER) return laxMakeError("zero: not a number");
    return laxMakeBool(v->numVal==0);
}

LaxValue* laxBuiltinGcd(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args)!=2) return laxMakeError("gcd needs 2 args");
    LaxValue* a=laxEval(args->pairVal.car,env); if(a->type==LAX_ERROR)return a;
    LaxValue* b=laxEval(args->pairVal.cdr->pairVal.car,env); if(b->type==LAX_ERROR)return b;
    if(a->type!=LAX_NUMBER||b->type!=LAX_NUMBER) return laxMakeError("gcd: not numbers");
    long x=(long)(a->numVal<0?-a->numVal:a->numVal), y=(long)(b->numVal<0?-b->numVal:b->numVal);
    while(y){long t=y;y=x%y;x=t;}
    return laxMakeNumber((double)x);
}

LaxValue* laxBuiltinLcm(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args)!=2) return laxMakeError("lcm needs 2 args");
    LaxValue* a=laxEval(args->pairVal.car,env); if(a->type==LAX_ERROR)return a;
    LaxValue* b=laxEval(args->pairVal.cdr->pairVal.car,env); if(b->type==LAX_ERROR)return b;
    if(a->type!=LAX_NUMBER||b->type!=LAX_NUMBER) return laxMakeError("lcm: not numbers");
    long x=(long)(a->numVal<0?-a->numVal:a->numVal), y=(long)(b->numVal<0?-b->numVal:b->numVal);
    long g=x,ty=y; while(ty){long t=ty;ty=g%ty;g=t;}
    return laxMakeNumber(g==0?0:(double)(x/g*y));
}

LaxValue* laxBuiltinFact(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args)!=1) return laxMakeError("fact needs 1 arg");
    LaxValue* v=laxEval(args->pairVal.car,env); if(v->type==LAX_ERROR)return v;
    if(v->type!=LAX_NUMBER) return laxMakeError("fact: not a number");
    int n=(int)v->numVal;
    if (n < 0) return laxMakeError("fact: negative");
    if (n > 20) return laxMakeError("fact: too large (max 20)");
    double r=1; for(int i=2;i<=n;i++) r*=i;
    return laxMakeNumber(r);
}

LaxValue* laxBuiltinFib(LaxValue* args, LaxEnvironment* env) {
    if (laxListLength(args)!=1) return laxMakeError("fib needs 1 arg");
    LaxValue* v=laxEval(args->pairVal.car,env); if(v->type==LAX_ERROR)return v;
    if(v->type!=LAX_NUMBER) return laxMakeError("fib: not a number");
    int n=(int)v->numVal;
    if (n < 0) return laxMakeError("fib: negative");
    if (n > 50) return laxMakeError("fib: too large (max 50)");
    double a=0,b=1; for(int i=0;i<n;i++){double t=a+b;a=b;b=t;}
    return laxMakeNumber(a);
}

LaxValue* laxBuiltinSumN(LaxValue* args, LaxEnvironment* env) {
    double s=0;
    while(args->type==LAX_PAIR){LaxValue* v=laxEval(args->pairVal.car,env);if(v->type==LAX_ERROR)return v;if(v->type!=LAX_NUMBER)return laxMakeError("sum: not a number");s+=v->numVal;args=args->pairVal.cdr;}
    return laxMakeNumber(s);
}

LaxValue* laxBuiltinAvg(LaxValue* args, LaxEnvironment* env) {
    double s=0; int c=0;
    while(args->type==LAX_PAIR){LaxValue* v=laxEval(args->pairVal.car,env);if(v->type==LAX_ERROR)return v;if(v->type!=LAX_NUMBER)return laxMakeError("avg: not a number");s+=v->numVal;c++;args=args->pairVal.cdr;}
    if(c==0) return laxMakeError("avg: no arguments");
    return laxMakeNumber(s/c);
}

LaxValue* laxBuiltinInc(LaxValue* args, LaxEnvironment* env) {
    if(laxListLength(args)!=1) return laxMakeError("inc needs 1 arg");
    LaxValue* v=laxEval(args->pairVal.car,env); if(v->type==LAX_ERROR)return v;
    if(v->type!=LAX_NUMBER) return laxMakeError("inc: not a number");
    return laxMakeNumber(v->numVal+1);
}
LaxValue* laxBuiltinDec(LaxValue* args, LaxEnvironment* env) {
    if(laxListLength(args)!=1) return laxMakeError("dec needs 1 arg");
    LaxValue* v=laxEval(args->pairVal.car,env); if(v->type==LAX_ERROR)return v;
    if(v->type!=LAX_NUMBER) return laxMakeError("dec: not a number");
    return laxMakeNumber(v->numVal-1);
}
LaxValue* laxBuiltinHalf(LaxValue* args, LaxEnvironment* env) {
    if(laxListLength(args)!=1) return laxMakeError("half needs 1 arg");
    LaxValue* v=laxEval(args->pairVal.car,env); if(v->type==LAX_ERROR)return v;
    if(v->type!=LAX_NUMBER) return laxMakeError("half: not a number");
    return laxMakeNumber(v->numVal/2.0);
}
LaxValue* laxBuiltinDouble(LaxValue* args, LaxEnvironment* env) {
    if(laxListLength(args)!=1) return laxMakeError("double needs 1 arg");
    LaxValue* v=laxEval(args->pairVal.car,env); if(v->type==LAX_ERROR)return v;
    if(v->type!=LAX_NUMBER) return laxMakeError("double: not a number");
    return laxMakeNumber(v->numVal*2.0);
}
LaxValue* laxBuiltinPercent(LaxValue* args, LaxEnvironment* env) {
    if(laxListLength(args)!=2) return laxMakeError("percent needs 2 args");
    LaxValue* a=laxEval(args->pairVal.car,env); if(a->type==LAX_ERROR)return a;
    LaxValue* b=laxEval(args->pairVal.cdr->pairVal.car,env); if(b->type==LAX_ERROR)return b;
    if(a->type!=LAX_NUMBER||b->type!=LAX_NUMBER) return laxMakeError("percent: not numbers");
    if(b->numVal==0) return laxMakeError("percent: total is zero");
    return laxMakeNumber(a->numVal/b->numVal*100.0);
}

LaxValue* laxBuiltinStrContains(LaxValue* args, LaxEnvironment* env) {
    if(laxListLength(args)!=2) return laxMakeError("contains needs 2 args");
    LaxValue* s=laxEval(args->pairVal.car,env); if(s->type==LAX_ERROR)return s;
    LaxValue* p=laxEval(args->pairVal.cdr->pairVal.car,env); if(p->type==LAX_ERROR)return p;
    if(s->type!=LAX_STRING||p->type!=LAX_STRING) return laxMakeError("contains: not strings");
    size_t sl=s->strVal.length, pl=p->strVal.length;
    if (pl == 0) return laxMakeBool(true);
    if (pl > sl) return laxMakeBool(false);
    for(size_t i=0;i<=sl-pl;i++){bool m=true;for(size_t j=0;j<pl;j++){if(s->strVal.data[i+j]!=p->strVal.data[j]){m=false;break;}}if(m)return laxMakeBool(true);}
    return laxMakeBool(false);
}

LaxValue* laxBuiltinStrRepeat(LaxValue* args, LaxEnvironment* env) {
    if(laxListLength(args)!=2) return laxMakeError("strrepeat needs 2 args");
    LaxValue* s=laxEval(args->pairVal.car,env); if(s->type==LAX_ERROR)return s;
    LaxValue* c=laxEval(args->pairVal.cdr->pairVal.car,env); if(c->type==LAX_ERROR)return c;
    if(s->type!=LAX_STRING) return laxMakeError("strrepeat: not a string");
    if(c->type!=LAX_NUMBER) return laxMakeError("strrepeat: not a number");
    int n=(int)c->numVal; if(n<0)n=0; if(n>100)n=100;
    char buf[4096]; buf[0]='\0';
    for(int i=0;i<n&&EsCStringLength(buf)+s->strVal.length<4090;i++) EsCRTstrcat(buf,s->strVal.data);
    return laxMakeString(buf);
}

void laxRegisterBuiltins(LaxEnvironment* env) {
    // I/O
    laxDefine(env, "image",    laxMakeProcedure(laxBuiltinImage,    "image"));
    laxDefine(env, "print",    laxMakeProcedure(laxBuiltinPrint,    "print"));
    laxDefine(env, "input",    laxMakeProcedure(laxBuiltinInput,    "input"));
    // Core math
    laxDefine(env, "sqrt",     laxMakeProcedure(laxBuiltinSqrt,     "sqrt"));
    laxDefine(env, "sqr",      laxMakeProcedure(laxBuiltinSqr,      "sqr"));
    laxDefine(env, "abs",      laxMakeProcedure(laxBuiltinAbs,      "abs"));
    laxDefine(env, "pow",      laxMakeProcedure(laxBuiltinPow,      "pow"));
    laxDefine(env, "max",      laxMakeProcedure(laxBuiltinMax,      "max"));
    laxDefine(env, "min",      laxMakeProcedure(laxBuiltinMin,      "min"));
    laxDefine(env, "floor",    laxMakeProcedure(laxBuiltinFloor,    "floor"));
    laxDefine(env, "ceil",     laxMakeProcedure(laxBuiltinCeil,     "ceil"));
    laxDefine(env, "round",    laxMakeProcedure(laxBuiltinRound,    "round"));
    laxDefine(env, "mod",      laxMakeProcedure(laxBuiltinMod,      "mod"));
    laxDefine(env, "neg",      laxMakeProcedure(laxBuiltinNeg,      "neg"));
    laxDefine(env, "sign",     laxMakeProcedure(laxBuiltinSign,     "sign"));
    laxDefine(env, "inc",      laxMakeProcedure(laxBuiltinInc,      "inc"));
    laxDefine(env, "dec",      laxMakeProcedure(laxBuiltinDec,      "dec"));
    laxDefine(env, "half",     laxMakeProcedure(laxBuiltinHalf,     "half"));
    laxDefine(env, "double",   laxMakeProcedure(laxBuiltinDouble,   "double"));
    laxDefine(env, "clamp",    laxMakeProcedure(laxBuiltinClamp,    "clamp"));
    laxDefine(env, "lerp",     laxMakeProcedure(laxBuiltinLerp,     "lerp"));
    laxDefine(env, "percent",  laxMakeProcedure(laxBuiltinPercent,  "percent"));
    laxDefine(env, "sum",      laxMakeProcedure(laxBuiltinSumN,     "sum"));
    laxDefine(env, "avg",      laxMakeProcedure(laxBuiltinAvg,      "avg"));
    laxDefine(env, "gcd",      laxMakeProcedure(laxBuiltinGcd,      "gcd"));
    laxDefine(env, "lcm",      laxMakeProcedure(laxBuiltinLcm,      "lcm"));
    laxDefine(env, "fact",     laxMakeProcedure(laxBuiltinFact,     "fact"));
    laxDefine(env, "fib",      laxMakeProcedure(laxBuiltinFib,      "fib"));
    // Trig / advanced
    laxDefine(env, "sin",      laxMakeProcedure(laxBuiltinSin,      "sin"));
    laxDefine(env, "cos",      laxMakeProcedure(laxBuiltinCos,      "cos"));
    laxDefine(env, "tan",      laxMakeProcedure(laxBuiltinTan,      "tan"));
    laxDefine(env, "log",      laxMakeProcedure(laxBuiltinLog,      "log"));
    laxDefine(env, "log10",    laxMakeProcedure(laxBuiltinLog10,    "log10"));
    laxDefine(env, "exp",      laxMakeProcedure(laxBuiltinExp,      "exp"));
    // Logic
    laxDefine(env, "not",      laxMakeProcedure(laxBuiltinNot,      "not"));
    laxDefine(env, "and",      laxMakeProcedure(laxBuiltinAnd,      "and"));
    laxDefine(env, "or",       laxMakeProcedure(laxBuiltinOr,       "or"));
    // Predicates
    laxDefine(env, "isnum",    laxMakeProcedure(laxBuiltinIsNum,    "isnum"));
    laxDefine(env, "isstr",    laxMakeProcedure(laxBuiltinIsStr,    "isstr"));
    laxDefine(env, "isbool",   laxMakeProcedure(laxBuiltinIsBool,   "isbool"));
    laxDefine(env, "even",     laxMakeProcedure(laxBuiltinEven,     "even"));
    laxDefine(env, "odd",      laxMakeProcedure(laxBuiltinOdd,      "odd"));
    laxDefine(env, "positive", laxMakeProcedure(laxBuiltinPositive, "positive"));
    laxDefine(env, "negative", laxMakeProcedure(laxBuiltinNegative, "negative"));
    laxDefine(env, "zero",     laxMakeProcedure(laxBuiltinZero,     "zero"));
    // Strings
    laxDefine(env, "strlen",   laxMakeProcedure(laxBuiltinStrlen,   "strlen"));
    laxDefine(env, "strcat",   laxMakeProcedure(laxBuiltinStrcat,   "strcat"));
    laxDefine(env, "numstr",   laxMakeProcedure(laxBuiltinNumToStr, "numstr"));
    laxDefine(env, "contains", laxMakeProcedure(laxBuiltinStrContains,"contains"));
    laxDefine(env, "strrepeat",laxMakeProcedure(laxBuiltinStrRepeat,"strrepeat"));
    // Constants
    laxDefine(env, "true",     laxMakeBool(true));
    laxDefine(env, "false",    laxMakeBool(false));
    laxDefine(env, "pi",       laxMakeNumber(3.14159265358979323846));
    laxDefine(env, "e",        laxMakeNumber(2.71828182845904523536));
    laxDefine(env, "phi",      laxMakeNumber(1.61803398874989484820));
    laxDefine(env, "tau",      laxMakeNumber(6.28318530717958647692));
    laxDefine(env, "inf",      laxMakeNumber(1.0 / 0.0));
    laxDefine(env, "-inf",     laxMakeNumber(-1.0 / 0.0));
}

LaxValue* laxCheckInfix(LaxValue* list, LaxEnvironment* env) {
    if (list->type != LAX_PAIR) return nullptr;
    int len = laxListLength(list);
    
    if (len == 2) {
        LaxValue* first = list->pairVal.car;
        LaxValue* arg = list->pairVal.cdr->pairVal.car;
        
        if (first->type == LAX_SYMBOL) {
            LaxValue* proc = laxLookup(env, first->strVal.data);
            if (proc && proc->type == LAX_PROCEDURE && proc->procVal.proc) {
                LaxValue* argList = laxMakePair(arg, laxMakeNil());
                return proc->procVal.proc(argList, env);
            }
        }
    }
    
    if (len != 3) return nullptr;
    
    LaxValue* left = list->pairVal.car;
    LaxValue* op = list->pairVal.cdr->pairVal.car;
    LaxValue* right = list->pairVal.cdr->pairVal.cdr->pairVal.car;
    
    if (op->type != LAX_SYMBOL) return nullptr;
    const char* opStr = op->strVal.data;
    
    if (EsCRTstrcmp(opStr, "+") != 0 && EsCRTstrcmp(opStr, "-") != 0 && 
        EsCRTstrcmp(opStr, "*") != 0 && EsCRTstrcmp(opStr, "/") != 0 &&
        EsCRTstrcmp(opStr, "\\") != 0 &&
        EsCRTstrcmp(opStr, "=") != 0 && EsCRTstrcmp(opStr, "<") != 0 && 
        EsCRTstrcmp(opStr, ">") != 0 && EsCRTstrcmp(opStr, "<=") != 0 &&
        EsCRTstrcmp(opStr, ">=") != 0 && EsCRTstrcmp(opStr, "!=") != 0) {
        return nullptr;
    }
    
    LaxValue* leftVal = laxEval(left, env);
    if (leftVal->type == LAX_ERROR) return leftVal;
    LaxValue* rightVal = laxEval(right, env);
    if (rightVal->type == LAX_ERROR) return rightVal;
    
    if (leftVal->type != LAX_NUMBER || rightVal->type != LAX_NUMBER) {
        return laxMakeError("Operator needs numbers");
    }
    
    if (EsCRTstrcmp(opStr, "+") == 0) return laxMakeNumber(leftVal->numVal + rightVal->numVal);
    if (EsCRTstrcmp(opStr, "-") == 0) return laxMakeNumber(leftVal->numVal - rightVal->numVal);
    if (EsCRTstrcmp(opStr, "*") == 0) return laxMakeNumber(leftVal->numVal * rightVal->numVal);
    if (EsCRTstrcmp(opStr, "/") == 0 || EsCRTstrcmp(opStr, "\\") == 0) {
        if (rightVal->numVal == 0) {
            if (leftVal->numVal == 0) return laxMakeNumber(0.0 / 0.0); // nan
            return laxMakeNumber(leftVal->numVal > 0 ? 1.0/0.0 : -1.0/0.0); // +-inf
        }
        return laxMakeNumber(leftVal->numVal / rightVal->numVal);
    }
    if (EsCRTstrcmp(opStr, "=") == 0) return laxMakeBool(leftVal->numVal == rightVal->numVal);
    if (EsCRTstrcmp(opStr, "<") == 0) return laxMakeBool(leftVal->numVal < rightVal->numVal);
    if (EsCRTstrcmp(opStr, ">") == 0) return laxMakeBool(leftVal->numVal > rightVal->numVal);
    if (EsCRTstrcmp(opStr, "<=") == 0) return laxMakeBool(leftVal->numVal <= rightVal->numVal);
    if (EsCRTstrcmp(opStr, ">=") == 0) return laxMakeBool(leftVal->numVal >= rightVal->numVal);
    if (EsCRTstrcmp(opStr, "!=") == 0) return laxMakeBool(leftVal->numVal != rightVal->numVal);
    
    return nullptr;
}

LaxValue* laxEval(LaxValue* expr, LaxEnvironment* env) {
    if (expr->type == LAX_NUMBER || expr->type == LAX_STRING || 
        expr->type == LAX_BOOL || expr->type == LAX_NIL) {
        return expr;
    }
    if (expr->type == LAX_SYMBOL) {
        if (EsCRTstrcmp(expr->strVal.data, ".") == 0) {
            // [.] as symbol → used inside expressions, returns true silently
            return laxMakeBool(true);
        }
        if (EsCRTstrcmp(expr->strVal.data, "..") == 0) {
            return laxMakeBool(true);
        }
        LaxValue* val = laxLookup(env, expr->strVal.data);
        if (val) return val;
        char buffer[512];
        EsCRTsnprintf(buffer, sizeof(buffer), "Undefined variable: '%s' (not a command or value)", expr->strVal.data);
        return laxMakeError(buffer);
    }
    if (expr->type == LAX_PAIR) {
        int len = laxListLength(expr);
        LaxValue* first = expr->pairVal.car;
        
        // [.] → print #t and return true
        if (len == 1 && first->type == LAX_SYMBOL && EsCRTstrcmp(first->strVal.data, ".") == 0) {
            AddOutput(scriptInstance, "#t\n", 3);
            return laxMakeBool(true);
        }
        // [..] → return true silently
        if (len == 1 && first->type == LAX_SYMBOL && EsCRTstrcmp(first->strVal.data, "..") == 0) {
            return laxMakeBool(true);
        }
        // [x] where x is not a symbol → just return evaluated x
        if (len == 1 && first->type != LAX_SYMBOL) {
            return laxEval(first, env);
        }
        // [x] where x is a symbol but NOT a function → just return its value
        if (len == 1 && first->type == LAX_SYMBOL) {
            LaxValue* val = laxLookup(env, first->strVal.data);
            if (val && val->type != LAX_PROCEDURE) return val;
            // if it's a procedure or undefined, fall through to normal eval
        }
        
        LaxValue* infixResult = laxCheckInfix(expr, env);
        if (infixResult) return infixResult;
        
        if (first->type == LAX_SYMBOL) {
            const char* sym = first->strVal.data;
            
            if (EsCRTstrcmp(sym, "spot") == 0) {
                LaxValue* args = expr->pairVal.cdr;
                if (laxListLength(args) < 2) return laxMakeError("spot: needs 2 args");
                LaxValue* name = args->pairVal.car;
                if (name->type != LAX_SYMBOL) return laxMakeError("spot: name must be symbol");
                LaxValue* value = laxEval(args->pairVal.cdr->pairVal.car, env);
                if (value->type == LAX_ERROR) return value;
                laxDefine(env, name->strVal.data, value);
                return laxMakeNil();
            }
            
            if (EsCRTstrcmp(sym, "if") == 0) {
                LaxValue* args = expr->pairVal.cdr;
                int argLen = laxListLength(args);
                if (argLen < 2) return laxMakeError("if: needs at least 2 args");
                
                LaxValue* cond;
                LaxValue* thenBranch;
                LaxValue* elseBranch = nullptr;
                
                // Check for [if a op b then else] - infix condition (5 or 6 args)
                if (argLen >= 4) {
                    LaxValue* a1 = args->pairVal.car;
                    LaxValue* a2 = args->pairVal.cdr->pairVal.car;
                    // If second arg looks like an operator, treat as infix condition
                    if (a2->type == LAX_SYMBOL) {
                        const char* op = a2->strVal.data;
                        if (EsCRTstrcmp(op,"=")==0 || EsCRTstrcmp(op,"<")==0 ||
                            EsCRTstrcmp(op,">")==0 || EsCRTstrcmp(op,"<=")==0 ||
                            EsCRTstrcmp(op,">=")==0 || EsCRTstrcmp(op,"!=")==0) {
                            // Build [a1 op a3] and evaluate as infix condition
                            LaxValue* a3 = args->pairVal.cdr->pairVal.cdr->pairVal.car;
                            LaxValue* condList = laxMakePair(a1, laxMakePair(a2, laxMakePair(a3, laxMakeNil())));
                            cond = laxCheckInfix(condList, env);
                            if (!cond) cond = laxMakeError("if: bad infix condition");
                            thenBranch = args->pairVal.cdr->pairVal.cdr->pairVal.cdr->pairVal.car;
                            if (argLen >= 5)
                                elseBranch = args->pairVal.cdr->pairVal.cdr->pairVal.cdr->pairVal.cdr->pairVal.car;
                            if (cond->type == LAX_ERROR) return cond;
                            if (laxIsTruthy(cond)) return laxEval(thenBranch, env);
                            if (elseBranch) return laxEval(elseBranch, env);
                            return laxMakeNil();
                        }
                    }
                }
                
                // Normal: [if condition then else]
                cond = laxEval(args->pairVal.car, env);
                if (cond->type == LAX_ERROR) return cond;
                thenBranch = args->pairVal.cdr->pairVal.car;
                if (laxIsTruthy(cond)) {
                    return laxEval(thenBranch, env);
                } else if (argLen >= 3) {
                    return laxEval(args->pairVal.cdr->pairVal.cdr->pairVal.car, env);
                }
                return laxMakeNil();
            }
            
            if (EsCRTstrcmp(sym, "start") == 0) {
                LaxValue* args = expr->pairVal.cdr;
                LaxValue* result = laxMakeNil();
                while (args->type == LAX_PAIR) {
                    result = laxEval(args->pairVal.car, env);
                    if (result->type == LAX_ERROR) return result;
                    args = args->pairVal.cdr;
                }
                return result;
            }
            
            if (EsCRTstrcmp(sym, "repeat") == 0) {
                LaxValue* args = expr->pairVal.cdr;
                if (laxListLength(args) < 2) return laxMakeError("repeat: needs count body");
                LaxValue* countVal = laxEval(args->pairVal.car, env);
                if (countVal->type == LAX_ERROR) return countVal;
                if (countVal->type != LAX_NUMBER) return laxMakeError("repeat: count must be number");
                int count = (int)countVal->numVal;
                LaxValue* body = args->pairVal.cdr->pairVal.car;
                LaxValue* result = laxMakeNil();
                for (int i = 0; i < count && i < 10000; i++) {
                    laxDefine(env, "i", laxMakeNumber((double)i));
                    result = laxEval(body, env);
                    if (result->type == LAX_ERROR) return result;
                }
                return result;
            }
        }
        
        LaxValue* proc = laxEval(first, env);
        if (proc->type == LAX_ERROR) return proc;
        if (proc->type == LAX_PROCEDURE) {
            LaxValue* args = expr->pairVal.cdr;
            return proc->procVal.proc(args, env);
        }
        char buf[256];
        EsCRTsnprintf(buf, sizeof(buf), "Not a function: '%s'", 
            first->type == LAX_SYMBOL ? first->strVal.data : "value");
        return laxMakeError(buf);
    }
    return laxMakeError("Cannot evaluate");
}

LaxValue* laxParseOne(const char* input, size_t* posPtr);

LaxValue* laxParseOne(const char* input, size_t* posPtr) {
    size_t len = EsCStringLength(input);
    size_t pos = *posPtr;
    
    while (pos < len && (input[pos] == ' ' || input[pos] == '\t' || input[pos] == '\n')) pos++;
    if (pos >= len) { *posPtr = pos; return laxMakeNil(); }
    
    if (input[pos] != '[') {
        *posPtr = pos;
        return laxMakeError("Input must start with [");
    }
    
    pos++;
    LaxValue* result = laxMakeNil();
    LaxValue* tail = nullptr;
    
    while (pos < len) {
        while (pos < len && (input[pos] == ' ' || input[pos] == '\t' || input[pos] == '\n')) pos++;
        if (pos >= len) { *posPtr = pos; return laxMakeError("Missing ]"); }
        if (input[pos] == ']') { pos++; *posPtr = pos; return result; }
        
        LaxValue* val = nullptr;
        
        if (input[pos] == '[') {
            val = laxParseOne(input, &pos);
            if (val->type == LAX_ERROR) { *posPtr = pos; return val; }
        } else if (input[pos] == '"' || input[pos] == '\'' || 
                   (unsigned char)input[pos] == 0xE2) {
            bool isQuote = false;
            
            if (input[pos] == '"' || input[pos] == '\'') {
                isQuote = true;
            } else if ((unsigned char)input[pos] == 0xE2 && pos + 2 < len) {
                if ((unsigned char)input[pos+1] == 0x80 && 
                    ((unsigned char)input[pos+2] == 0x9C || (unsigned char)input[pos+2] == 0x9D ||
                     (unsigned char)input[pos+2] == 0x98 || (unsigned char)input[pos+2] == 0x99)) {
                    isQuote = true;
                    pos += 2;
                }
            }
            
            if (isQuote) {
                pos++;
                char str[4096];
                size_t strLen = 0;
                bool foundClosing = false;
                
                while (pos < len && strLen < 4090) {
                    if (input[pos] == '"' || input[pos] == '\'') {
                        foundClosing = true;
                        pos++;
                        break;
                    }
                    if ((unsigned char)input[pos] == 0xE2 && pos + 2 < len && (unsigned char)input[pos+1] == 0x80) {
                        if ((unsigned char)input[pos+2] == 0x9C || (unsigned char)input[pos+2] == 0x9D ||
                            (unsigned char)input[pos+2] == 0x98 || (unsigned char)input[pos+2] == 0x99) {
                            foundClosing = true;
                            pos += 3;
                            break;
                        }
                    }
                    
                    if (input[pos] == '\\' && pos + 1 < len) {
                        pos++;
                        if (input[pos] == 'n') str[strLen++] = '\n';
                        else if (input[pos] == 't') str[strLen++] = '\t';
                        else if (input[pos] == 'r') str[strLen++] = '\r';
                        else if (input[pos] == '\\') str[strLen++] = '\\';
                        else if (input[pos] == '"') str[strLen++] = '"';
                        else if (input[pos] == '\'') str[strLen++] = '\'';
                        else str[strLen++] = input[pos];
                        pos++;
                    } else {
                        str[strLen++] = input[pos];
                        pos++;
                    }
                }
                
                if (!foundClosing) {
                    *posPtr = pos;
                    return laxMakeError("Unclosed string - missing closing quote");
                }
                
                str[strLen] = '\0';
                val = laxMakeString(str);
            } else {
                char token[256];
                size_t tokenLen = 0;
                while (pos < len && input[pos] != ' ' && input[pos] != '\t' && 
                       input[pos] != '\n' && input[pos] != '[' && input[pos] != ']' && tokenLen < 255) {
                    token[tokenLen++] = input[pos++];
                }
                token[tokenLen] = '\0';
                
                if (tokenLen > 0) {
                    bool isNumber = true;
                    bool hasDot = false;
                    bool hasDigit = false;
                    for (size_t i = 0; i < tokenLen; i++) {
                        if (i == 0 && (token[i] == '-' || token[i] == '+')) continue;
                        if (token[i] == '.' && !hasDot) { hasDot = true; continue; }
                        if (token[i] >= '0' && token[i] <= '9') { hasDigit = true; continue; }
                        isNumber = false; break;
                    }
                    isNumber = isNumber && hasDigit;
                    if (isNumber) {
                        double num = 0;
                        int sign = 1;
                        size_t i = 0;
                        if (token[0] == '-') { sign = -1; i = 1; }
                        else if (token[0] == '+') i = 1;
                        while (i < tokenLen && token[i] != '.') {
                            num = num * 10 + (token[i] - '0');
                            i++;
                        }
                        if (i < tokenLen && token[i] == '.') {
                            i++;
                            double frac = 0, div = 10;
                            while (i < tokenLen) {
                                frac += (token[i] - '0') / div;
                                div *= 10;
                                i++;
                            }
                            num += frac;
                        }
                        val = laxMakeNumber(sign * num);
                    } else if (EsCRTstrcmp(token, "true") == 0) {
                        val = laxMakeBool(true);
                    } else if (EsCRTstrcmp(token, "false") == 0) {
                        val = laxMakeBool(false);
                    } else {
                        val = laxMakeSymbol(token);
                    }
                }
            }
        } else {
            char token[256];
            size_t tokenLen = 0;
            while (pos < len && input[pos] != ' ' && input[pos] != '\t' && 
                   input[pos] != '\n' && input[pos] != '[' && input[pos] != ']' && tokenLen < 255) {
                token[tokenLen++] = input[pos++];
            }
            token[tokenLen] = '\0';
            
            if (tokenLen > 0) {
                bool isNumber = true;
                bool hasDot = false;
                for (size_t i = 0; i < tokenLen; i++) {
                    if (i == 0 && (token[i] == '-' || token[i] == '+')) continue;
                    if (token[i] == '.' && !hasDot) { hasDot = true; continue; }
                    if (token[i] < '0' || token[i] > '9') { isNumber = false; break; }
                }
                
                if (isNumber && tokenLen > 0 && !(tokenLen == 1 && (token[0] == '-' || token[0] == '+'))) {
                    double num = 0;
                    int sign = 1;
                    size_t i = 0;
                    if (token[0] == '-') { sign = -1; i = 1; }
                    else if (token[0] == '+') i = 1;
                    while (i < tokenLen && token[i] != '.') {
                        num = num * 10 + (token[i] - '0');
                        i++;
                    }
                    if (i < tokenLen && token[i] == '.') {
                        i++;
                        double frac = 0, div = 10;
                        while (i < tokenLen) {
                            frac += (token[i] - '0') / div;
                            div *= 10;
                            i++;
                        }
                        num += frac;
                    }
                    val = laxMakeNumber(sign * num);
                } else if (EsCRTstrcmp(token, "true") == 0) {
                    val = laxMakeBool(true);
                } else if (EsCRTstrcmp(token, "false") == 0) {
                    val = laxMakeBool(false);
                } else {
                    val = laxMakeSymbol(token);
                }
            }
        }
        
        if (val) {
            LaxValue* newPair = laxMakePair(val, laxMakeNil());
            if (result->type == LAX_NIL) {
                result = newPair;
                tail = newPair;
            } else {
                tail->pairVal.cdr = newPair;
                tail = newPair;
            }
        }
    }
    
    *posPtr = pos;
    return laxMakeError("Unclosed [");
}

LaxValue* laxParse(const char* input) {
    size_t pos = 0;
    LaxValue* commands = laxMakeNil();
    LaxValue* tail = nullptr;
    
    while (true) {
        LaxValue* cmd = laxParseOne(input, &pos);
        if (cmd->type == LAX_ERROR) return cmd;
        if (cmd->type == LAX_NIL) break;
        
        LaxValue* newPair = laxMakePair(cmd, laxMakeNil());
        if (commands->type == LAX_NIL) {
            commands = newPair;
            tail = newPair;
        } else {
            tail->pairVal.cdr = newPair;
            tail = newPair;
        }
    }
    
    if (laxListLength(commands) == 0) return laxMakeNil();
    if (laxListLength(commands) == 1) return commands->pairVal.car;
    
    LaxValue* startCmd = laxMakePair(laxMakeSymbol("start"), commands);
    return startCmd;
}

void ScriptThread(EsGeneric _instance) {
    Instance *instance = (Instance *) _instance.p;
    scriptInstance = instance;
    
    if (instance->executionLimitReached) {
        return;
    }
    
    instance->executionCount++;
    
    if (instance->executionCount >= 50) {
        instance->executionLimitReached = true;
        AddOutput(instance, "Error: Execution limit reached (50 commands).\nPlease press Clear output to continue.\n", 91);
        instance->anyOutput = true;
        
        if (instance->outputLineBufferBytes > 0) {
            if (EsUTF8IsValid(instance->outputLineBuffer, instance->outputLineBufferBytes)) {
                EsMessageMutexAcquire();
                EsTextDisplayCreate(instance->outputPanel, LT_CELL_H_FILL | LT_TEXT_DISPLAY_RICH_TEXT,
                        EsStyleIntern(&styleOutputParagraph),
                        instance->outputLineBuffer, instance->outputLineBufferBytes);
                EsMessageMutexRelease();
            }
            instance->outputLineBufferBytes = 0;
        }
        
        EsMessageMutexAcquire();
        if (instance->outputPanel) {
            instance->outputElements.Add(EsElementGetLayoutParent(instance->outputPanel));
        }
        instance->outputPanel = nullptr;
        instance->outputDecoration = nullptr;
        
        if (instance->inputRow) {
            EsElementDestroy(instance->inputRow);
            instance->inputTextbox = nullptr;
            instance->inputRow = nullptr;
        }
        EsMessageMutexRelease();
        return;
    }
    
    LaxValue* parsed = laxParse(instance->inputText);
    int result = 0;
    
    if (parsed->type == LAX_ERROR) {
        AddOutput(instance, "Error: ", 7);
        AddOutput(instance, parsed->strVal.data, parsed->strVal.length);
        AddOutput(instance, "\n", 1);
        result = 1;
    } else {
        LaxValue* evalResult = laxEval(parsed, instance->globalEnv);
        if (evalResult->type == LAX_ERROR) {
            AddOutput(instance, "Error: ", 7);
            AddOutput(instance, evalResult->strVal.data, evalResult->strVal.length);
            AddOutput(instance, "\n", 1);
            result = 1;
        } else if (evalResult->type != LAX_NIL && instance->anyOutput == false) {
            if (evalResult->type == LAX_NUMBER) {
                char buffer[64];
                double n = evalResult->numVal;
                if (n != n) {
                    AddOutput(instance, "nan\n", 4);
                } else if (n > 1e308) {
                    AddOutput(instance, "inf\n", 4);
                } else if (n < -1e308) {
                    AddOutput(instance, "-inf\n", 5);
                } else {
                    EsCRTsnprintf(buffer, sizeof(buffer), "%g\n", n);
                    AddOutput(instance, buffer, EsCStringLength(buffer));
                }
            } else if (evalResult->type == LAX_STRING) {
                AddOutput(instance, "\"", 1);
                AddOutput(instance, evalResult->strVal.data, evalResult->strVal.length);
                AddOutput(instance, "\"\n", 2);
            }
        }
    }
    
    if (instance->outputLineBufferBytes > 0) {
        if (EsUTF8IsValid(instance->outputLineBuffer, instance->outputLineBufferBytes)) {
            EsMessageMutexAcquire();
            EsTextDisplayCreate(instance->outputPanel, LT_CELL_H_FILL | LT_TEXT_DISPLAY_RICH_TEXT,
                    EsStyleIntern(&styleOutputParagraph),
                    instance->outputLineBuffer, instance->outputLineBufferBytes);
            EsMessageMutexRelease();
        }
        instance->outputLineBufferBytes = 0;
    }
    
    instance->inputText = nullptr;
    
    EsMessageMutexAcquire();
    
    if (!instance->anyOutput) {
        EsElementDestroy(EsElementGetLayoutParent(instance->outputPanel));
        instance->outputPanel = nullptr;
    } else {
        instance->anyOutput = false;
    }
    
    instance->gotREPLResult = false;
    
    if (result == 0) {
        EsSpacerChangeStyle(instance->outputDecoration, EsStyleIntern(&styleOutputDecorationSuccess));
    } else {
        EsSpacerChangeStyle(instance->outputDecoration, EsStyleIntern(&styleOutputDecorationFailure));
    }
    
    instance->outputElements.Add(EsSpacerCreate(instance->root, LT_CELL_H_FILL, EsStyleIntern(&styleInterCommandSpacer)));
    AddPrompt(instance);
    
    EsMessageMutexRelease();
}

void EnterCommand(Instance *instance) {
    if (!instance->inputTextbox || !instance->inputRow) return;
    
    size_t dataBytes;
    char *data = EsTextboxGetContents(instance->inputTextbox, &dataBytes, LT_FLAGS_DEFAULT);
    
    if (instance->waitingForInput) {
        if (instance->inputResult) {
            EsHeapFree(instance->inputResult);
        }
        instance->inputResult = (char*)EsHeapAllocate(dataBytes + 1, false);
        EsMemoryCopy(instance->inputResult, data, dataBytes);
        instance->inputResult[dataBytes] = '\0';
        instance->inputReady = true;
        EsElementDestroy(instance->inputRow);
        instance->inputTextbox = nullptr;
        instance->inputRow = nullptr;
        EsHeapFree(data);
        return;
    }
    
    EsElementDestroy(instance->inputRow);
    instance->inputTextbox = nullptr;
    instance->inputRow = nullptr;
    
    uint8_t newline = '\n';
    EsFileWriteSync(globalCommandHistoryFile.handle, EsFileGetSize(globalCommandHistoryFile.handle), dataBytes, data);
    EsFileWriteSync(globalCommandHistoryFile.handle, EsFileGetSize(globalCommandHistoryFile.handle), 1, &newline);
    EsFileControl(globalCommandHistoryFile.handle, LT_FILE_CONTROL_FLUSH, nullptr, 0);
    
    EsPanel *commandLogRow = EsPanelCreate(instance->root, LT_CELL_H_FILL | LT_PANEL_STACK | LT_PANEL_HORIZONTAL, EsStyleIntern(&styleInputRow));
    EsTextDisplayCreate(commandLogRow, LT_FLAGS_DEFAULT, EsStyleIntern(&stylePromptText), "\u2661");
    EsTextDisplayCreate(commandLogRow, LT_CELL_H_FILL, EsStyleIntern(&styleCommandLogText), data, dataBytes);
    instance->outputElements.Add(commandLogRow);
    
    EsPanel *outputPanelWrapper = EsPanelCreate(instance->root, LT_CELL_H_FILL | LT_PANEL_STACK | LT_PANEL_HORIZONTAL, EsStyleIntern(&styleOutputPanelWrapper));
    instance->outputDecoration = EsSpacerCreate(outputPanelWrapper, LT_CELL_V_FILL, EsStyleIntern(&styleOutputDecorationInProgress));
    instance->outputPanel = EsPanelCreate(outputPanelWrapper, LT_CELL_H_FILL | LT_PANEL_STACK | LT_PANEL_VERTICAL, EsStyleIntern(&styleOutputPanel));
    instance->logOutputGroup = instance->outputPanel;
    
    instance->inputText = data;
    instance->inputBytes = dataBytes;
    EsThreadCreate(ScriptThread, &instance->scriptThread, instance);
    EsHandleClose(instance->scriptThread.handle);
}

int InputTextboxMessage(EsElement *element, EsMessage *message) {
    if (message->type == LT_MSG_KEY_TYPED) {
        if (message->keyboard.scancode == LT_SCANCODE_ENTER) {
            EnterCommand(element->instance);
            return LT_HANDLED;
        }
    }
    return 0;
}

void AddOutput(Instance *instance, const char *text, size_t textBytes) {
    instance->anyOutput = true;
    for (uintptr_t i = 0; i < textBytes; i++) {
        if (text[i] == '\n') {
            if (EsUTF8IsValid(instance->outputLineBuffer, instance->outputLineBufferBytes)) {
                EsMessageMutexAcquire();
                EsTextDisplayCreate(instance->outputPanel, LT_CELL_H_FILL | LT_TEXT_DISPLAY_RICH_TEXT,
                        EsStyleIntern(&styleOutputParagraph),
                        instance->outputLineBuffer, instance->outputLineBufferBytes);
                EsMessageMutexRelease();
            }
            instance->outputLineBufferBytes = 0;
        } else {
            if (instance->outputLineBufferBytes == instance->outputLineBufferAllocated) {
                instance->outputLineBufferAllocated = (instance->outputLineBufferAllocated + 4) * 2;
                instance->outputLineBuffer = (char *) EsHeapReallocate(instance->outputLineBuffer,
                        instance->outputLineBufferAllocated, false);
            }
            instance->outputLineBuffer[instance->outputLineBufferBytes++] = text[i];
        }
    }
}

void AddPrompt(Instance *instance) {
    if (instance->executionLimitReached) return;
    
    if (instance->outputPanel) {
        instance->outputElements.Add(EsElementGetLayoutParent(instance->outputPanel));
    }
    instance->outputPanel = nullptr;
    instance->outputDecoration = nullptr;
    instance->inputTextbox = nullptr;
    instance->inputRow = nullptr;
    
    instance->inputRow = EsPanelCreate(instance->root, LT_CELL_H_FILL | LT_PANEL_STACK | LT_PANEL_HORIZONTAL, EsStyleIntern(&styleInputRow));
    EsTextDisplayCreate(instance->inputRow, LT_FLAGS_DEFAULT, EsStyleIntern(&stylePromptText), "\u2665");
    instance->inputTextbox = EsTextboxCreate(instance->inputRow, LT_CELL_H_FILL, EsStyleIntern(&styleInputTextbox));
    instance->inputTextbox->messageUser = InputTextboxMessage;
    EsElementFocus(instance->inputTextbox, LT_ELEMENT_FOCUS_ENSURE_VISIBLE);
}

void CommandClearOutput(Instance *instance, EsElement *, EsCommand *) {
    for (uintptr_t i = 0; i < instance->outputElements.Length(); i++) {
        EsElementDestroy(instance->outputElements[i]);
    }
    instance->outputElements.Free();
    
    if (instance->inputRow) {
        EsElementDestroy(instance->inputRow);
        instance->inputTextbox = nullptr;
        instance->inputRow = nullptr;
    }
    instance->outputPanel = nullptr;
    instance->outputDecoration = nullptr;
    
    instance->executionCount = 0;
    instance->executionLimitReached = false;
    instance->waitingForInput = false;
    instance->inputReady = false;
    if (instance->inputResult) { EsHeapFree(instance->inputResult); instance->inputResult = nullptr; }
    if (instance->inputPrompt) { EsHeapFree(instance->inputPrompt); instance->inputPrompt = nullptr; }
    
    EsPanel *welcomeWrapper = EsPanelCreate(instance->root, LT_CELL_H_FILL | LT_PANEL_STACK | LT_PANEL_HORIZONTAL, EsStyleIntern(&styleOutputPanelWrapper));
    EsSpacerCreate(welcomeWrapper, LT_CELL_V_FILL, EsStyleIntern(&styleOutputDecorationSuccess));
    EsPanel *welcomePanel = EsPanelCreate(welcomeWrapper, LT_CELL_H_FILL | LT_PANEL_STACK | LT_PANEL_VERTICAL, EsStyleIntern(&styleOutputPanel));
    const char* welcomeMsg = "Lax Script Console\nVersion 1.0\n";
    EsTextDisplayCreate(welcomePanel, LT_CELL_H_FILL | LT_TEXT_DISPLAY_RICH_TEXT,
            EsStyleIntern(&styleOutputParagraph), welcomeMsg, EsCStringLength(welcomeMsg));
    instance->outputElements.Add(welcomeWrapper);
    instance->outputElements.Add(EsSpacerCreate(instance->root, LT_CELL_H_FILL, EsStyleIntern(&styleInterCommandSpacer)));
    
    instance->inputRow = EsPanelCreate(instance->root, LT_CELL_H_FILL | LT_PANEL_STACK | LT_PANEL_HORIZONTAL, EsStyleIntern(&styleInputRow));
    EsTextDisplayCreate(instance->inputRow, LT_FLAGS_DEFAULT, EsStyleIntern(&stylePromptText), "\u2665");
    instance->inputTextbox = EsTextboxCreate(instance->inputRow, LT_CELL_H_FILL, EsStyleIntern(&styleInputTextbox));
    instance->inputTextbox->messageUser = InputTextboxMessage;
    EsElementFocus(instance->inputTextbox, LT_ELEMENT_FOCUS_ENSURE_VISIBLE);
}

int InstanceCallback(Instance *instance, EsMessage *message) {
    if (message->type == LT_MSG_INSTANCE_DESTROY) {
        EsHeapFree(instance->outputLineBuffer);
        instance->outputElements.Free();
    }
    return 0;
}

void _start() {
    _init();
    globalCommandHistoryFile = EsFileOpen(EsLiteral("|Settings:/Command History.txt"), LT_FILE_WRITE);
    size_t deviceCount;
    EsMessageDevice *devices = EsDeviceEnumerate(&deviceCount);
    for (uintptr_t i = 0; i < deviceCount; i++) {
        if (devices[i].type == LT_DEVICE_FILE_SYSTEM && EsDeviceControl(devices[i].handle, LT_DEVICE_CONTROL_FS_IS_BOOT, 0, nullptr)) {
            EsMountPointAdd(EsLiteral("0:"), devices[i].handle);
        }
    }
    EsHeapFree(devices);
    while (true) {
        EsMessage *message = EsMessageReceive();
        if (message->type == LT_MSG_INSTANCE_CREATE) {
            Instance *instance = EsInstanceCreate(message, "Lax Script Console", -1);
            instance->callback = InstanceCallback;
            EsCommandRegister(&instance->commandClearOutput, instance, EsLiteral("Clear output"), CommandClearOutput,
                    1, "Ctrl+Shift+L", true);
            EsWindowSetIcon(instance->window, LT_ICON_UTILITIES_TERMINAL);
            EsPanel *wrapper = EsPanelCreate(instance->window, LT_CELL_FILL, LT_STYLE_PANEL_WINDOW_DIVIDER);
            EsPanel *background = EsPanelCreate(wrapper, LT_CELL_FILL | LT_PANEL_V_SCROLL_AUTO, EsStyleIntern(&styleBackground));
            instance->root = EsPanelCreate(background, LT_CELL_H_FILL | LT_PANEL_STACK | LT_PANEL_VERTICAL, EsStyleIntern(&styleRoot));
            instance->waitingForInput = false;
            instance->inputReady = false;
            instance->executionCount = 0;
            instance->executionLimitReached = false;
            instance->inputPrompt = nullptr;
            instance->inputResult = nullptr;
            
            instance->globalEnv = laxCreateEnvironment(nullptr);
            laxRegisterBuiltins(instance->globalEnv);
            
            EsPanel *welcomeWrapper = EsPanelCreate(instance->root, LT_CELL_H_FILL | LT_PANEL_STACK | LT_PANEL_HORIZONTAL, EsStyleIntern(&styleOutputPanelWrapper));
            EsSpacerCreate(welcomeWrapper, LT_CELL_V_FILL, EsStyleIntern(&styleOutputDecorationSuccess));
            EsPanel *welcomePanel = EsPanelCreate(welcomeWrapper, LT_CELL_H_FILL | LT_PANEL_STACK | LT_PANEL_VERTICAL, EsStyleIntern(&styleOutputPanel));
            
            const char* welcomeMsg = "Lax Script Console\nVersion 6.0.7\n";
            EsTextDisplayCreate(welcomePanel, LT_CELL_H_FILL | LT_TEXT_DISPLAY_RICH_TEXT,
                    EsStyleIntern(&styleOutputParagraph), welcomeMsg, EsCStringLength(welcomeMsg));
            
            instance->outputElements.Add(welcomeWrapper);
            instance->outputElements.Add(EsSpacerCreate(instance->root, LT_CELL_H_FILL, EsStyleIntern(&styleInterCommandSpacer)));
            
            AddPrompt(instance);
            EsElement *toolbar = EsWindowGetToolbar(instance->window);
            EsCommandAddButton(&instance->commandClearOutput,
                    EsButtonCreate(toolbar, LT_FLAGS_DEFAULT, 0, EsLiteral("Clear output")));
        }
    }
}
