#include <iostream> //TODO remove me for cout
#include <sstream>
#include "Exception.h"


std::ostream& operator<<(std::ostream& os, const CJavascriptException& ex)
{
  os << "JSError: " << ex.what();

  return os;
}

std::ostream& operator <<(std::ostream& os, const CJavascriptStackTrace& obj)
{
  obj.Dump(os);

  return os;
}

void CJavascriptException::Expose(void)
{
  std::cout << "Stubbed" << std::endl;
}

CJavascriptStackTracePtr CJavascriptStackTrace::GetCurrentStackTrace(
  v8::Isolate *isolate, int frame_limit, v8::StackTrace::StackTraceOptions options)
{
  v8::HandleScope handle_scope(isolate);

  v8::TryCatch try_catch(isolate);

  v8::Handle<v8::StackTrace> st = v8::StackTrace::CurrentStackTrace(isolate, frame_limit, options);

  if (st.IsEmpty()) CJavascriptException::ThrowIf(isolate, try_catch);

  return boost::shared_ptr<CJavascriptStackTrace>(new CJavascriptStackTrace(isolate, st));
}

CJavascriptStackFramePtr CJavascriptStackTrace::GetFrame(size_t idx) const
{
  v8::HandleScope handle_scope(m_isolate);

  v8::TryCatch try_catch(m_isolate);

  v8::Handle<v8::StackFrame> frame = Handle()->GetFrame(m_isolate, idx);

  if (frame.IsEmpty()) CJavascriptException::ThrowIf(m_isolate, try_catch);

  return boost::shared_ptr<CJavascriptStackFrame>(new CJavascriptStackFrame(m_isolate, frame));
}

void CJavascriptStackTrace::Dump(std::ostream& os) const
{
  v8::HandleScope handle_scope(m_isolate);

  v8::TryCatch try_catch(m_isolate);

  std::ostringstream oss;

  for (int i=0; i<GetFrameCount(); i++)
  {
    v8::Handle<v8::StackFrame> frame = GetFrame(i)->Handle();

    v8::String::Utf8Value funcName(m_isolate, frame->GetFunctionName()), scriptName(m_isolate, frame->GetScriptName());

    os << "\tat ";

    if (funcName.length())
      os << std::string(*funcName, funcName.length()) << " (";

    if (frame->IsEval())
    {
      os << "(eval)";
    }
    else
    {
      os << std::string(*scriptName, scriptName.length()) << ":"
          << frame->GetLineNumber() << ":" << frame->GetColumn();
    }

    if (funcName.length())
      os << ")";

    os << std::endl;
  }
}

const std::string CJavascriptStackFrame::GetScriptName() const
{
  v8::HandleScope handle_scope(m_isolate);

  v8::String::Utf8Value name(m_isolate, Handle()->GetScriptName());

  return std::string(*name, name.length());
}
const std::string CJavascriptStackFrame::GetFunctionName() const
{
  v8::HandleScope handle_scope(m_isolate);

  v8::String::Utf8Value name(m_isolate, Handle()->GetFunctionName());

  return std::string(*name, name.length());
}
const std::string CJavascriptException::GetName(void)
{
  if (m_exc.IsEmpty()) return std::string();

  assert(m_isolate->InContext());

  v8::HandleScope handle_scope(m_isolate);

  v8::String::Utf8Value msg(m_isolate, v8::Handle<v8::String>::Cast(Exception()->ToObject(m_isolate->GetCurrentContext()).ToLocalChecked()->Get(m_isolate->GetCurrentContext(), v8::String::NewFromUtf8(m_isolate, "name").ToLocalChecked()).ToLocalChecked()));

  return std::string(*msg, msg.length());
}
const std::string CJavascriptException::GetMessage(void)
{
  if (m_exc.IsEmpty()) return std::string();

  assert(m_isolate->InContext());

  v8::HandleScope handle_scope(m_isolate);

  v8::String::Utf8Value msg(m_isolate, v8::Handle<v8::String>::Cast(Exception()->ToObject(m_isolate->GetCurrentContext()).ToLocalChecked()->Get(m_isolate->GetCurrentContext(), v8::String::NewFromUtf8(m_isolate, "message").ToLocalChecked()).ToLocalChecked()));

  return std::string(*msg, msg.length());
}
const std::string CJavascriptException::GetScriptName(void)
{
  assert(m_isolate->InContext());

  v8::HandleScope handle_scope(m_isolate);

  if (!m_msg.IsEmpty() && !Message()->GetScriptResourceName().IsEmpty() &&
      !Message()->GetScriptResourceName()->IsUndefined())
  {
    v8::String::Utf8Value name(m_isolate, Message()->GetScriptResourceName());

    return std::string(*name, name.length());
  }

  return std::string();
}
int CJavascriptException::GetLineNumber(void)
{
  assert(m_isolate->InContext());

  v8::HandleScope handle_scope(m_isolate);

  return m_msg.IsEmpty() ? 1 : Message()->GetLineNumber(m_isolate->GetCurrentContext()).ToChecked();
}
int CJavascriptException::GetStartPosition(void)
{
  assert(m_isolate->InContext());

  v8::HandleScope handle_scope(m_isolate);

  return m_msg.IsEmpty() ? 1 : Message()->GetStartPosition();
}
int CJavascriptException::GetEndPosition(void)
{
  assert(m_isolate->InContext());

  v8::HandleScope handle_scope(m_isolate);

  return m_msg.IsEmpty() ? 1 : Message()->GetEndPosition();
}
int CJavascriptException::GetStartColumn(void)
{
  assert(m_isolate->InContext());

  v8::HandleScope handle_scope(m_isolate);

  return m_msg.IsEmpty() ? 1 : Message()->GetStartColumn();
}
int CJavascriptException::GetEndColumn(void)
{
  assert(m_isolate->InContext());

  v8::HandleScope handle_scope(m_isolate);

  return m_msg.IsEmpty() ? 1 : Message()->GetEndColumn();
}
const std::string CJavascriptException::GetSourceLine(void)
{
  assert(m_isolate->InContext());

  v8::HandleScope handle_scope(m_isolate);

  if (!m_msg.IsEmpty() && !Message()->GetSourceLine(m_isolate->GetCurrentContext()).IsEmpty())
  {
    v8::String::Utf8Value line(m_isolate, Message()->GetSourceLine(m_isolate->GetCurrentContext()).ToLocalChecked());

    return std::string(*line, line.length());
  }

  return std::string();
}
const std::string CJavascriptException::GetStackTrace(void)
{
  assert(m_isolate->InContext());

  v8::HandleScope handle_scope(m_isolate);

  if (!m_stack.IsEmpty())
  {
    v8::String::Utf8Value stack(m_isolate, v8::Handle<v8::String>::Cast(Stack()));

    return std::string(*stack, stack.length());
  }

  return std::string();
}
const std::string CJavascriptException::Extract(v8::Isolate *isolate, v8::TryCatch& try_catch)
{
  assert(isolate->InContext());

  v8::HandleScope handle_scope(isolate);

  std::ostringstream oss;

  v8::String::Utf8Value msg(isolate, try_catch.Exception());

  if (*msg)
    oss << std::string(*msg, msg.length());

  v8::Handle<v8::Message> message = try_catch.Message();

  if (!message.IsEmpty())
  {
    oss << " ( ";

    if (!message->GetScriptResourceName().IsEmpty() &&
        !message->GetScriptResourceName()->IsUndefined())
    {
      v8::String::Utf8Value name(isolate, message->GetScriptResourceName());

      oss << std::string(*name, name.length());
    }

    oss << " @ " << message->GetLineNumber(isolate->GetCurrentContext()).ToChecked()
        << " : " << message->GetStartColumn() << " ) ";

    if (!message->GetSourceLine(isolate->GetCurrentContext()).IsEmpty())
    {
      v8::String::Utf8Value line(isolate, message->GetSourceLine(isolate->GetCurrentContext()).ToLocalChecked());

      oss << " -> " << std::string(*line, line.length());
    }
  }

  return oss.str();
}

static struct {
  const char *name;
  PyObject *type;
} SupportErrors[] = {
  { "RangeError",     ::PyExc_IndexError },
  { "ReferenceError", ::PyExc_ReferenceError },
  { "SyntaxError",    ::PyExc_SyntaxError },
  { "TypeError",      ::PyExc_TypeError }
};

void CJavascriptException::ThrowIf(v8::Isolate *isolate, v8::TryCatch& try_catch)
{
  if (try_catch.HasCaught() && try_catch.CanContinue())
  {
    v8::HandleScope handle_scope(isolate);

    PyObject *type = NULL;
    v8::Handle<v8::Value> obj = try_catch.Exception();

    if (obj->IsObject())
    {
      v8::Handle<v8::Object> exc = obj->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
      v8::Handle<v8::String> name = v8::String::NewFromUtf8(isolate, "name").ToLocalChecked();

      if (exc->Has(isolate->GetCurrentContext(), name).ToChecked())
      {
        v8::String::Utf8Value s(isolate, v8::Handle<v8::String>::Cast(exc->Get(isolate->GetCurrentContext(), name).ToLocalChecked()));

        for (size_t i=0; i<_countof(SupportErrors); i++)
        {
          if (strnicmp(SupportErrors[i].name, *s, s.length()) == 0)
          {
            type = SupportErrors[i].type;
          }
        }
      }
    }

    throw CJavascriptException(isolate, try_catch, type);
  }
}


