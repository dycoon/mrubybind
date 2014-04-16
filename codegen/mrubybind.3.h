
namespace mrubybind {

//===========================================================================
// MrubyBind - utility class for binding C functions/classes to mruby.
class MrubyBind {
public:
  MrubyBind(mrb_state* mrb);
  MrubyBind(mrb_state* mrb, RClass* mod);
  ~MrubyBind();

  // Bind constant value.
  template <class T>
  void bind_const(const char* name, T v) {
    mrb_define_const(mrb_, mod_, name, Type<T>::ret(mrb_, v));
  }

  // Bind function.
  template <class Func>
  void bind(const char* func_name, Func func_ptr) {
    mrb_sym func_name_s = mrb_intern_cstr(mrb_, func_name);
    mrb_value env[] = {
      mrb_cptr_value(mrb_, (void*)func_ptr),  // 0: c function pointer
      mrb_symbol_value(func_name_s),          // 1: function name
    };
    struct RProc* proc = mrb_proc_new_cfunc_with_env(mrb_, Binder<Func>::call, 2, env);
    if (mod_ == mrb_->kernel_module)
      mrb_define_method_raw(mrb_, mod_, func_name_s, proc);
    else
      mrb_define_class_method_raw(mrb_, mod_, func_name_s, proc);
  }

  // Bind class.
  template <class Func>
  void bind_class(const char* class_name, Func new_func_ptr) {
    struct RClass *tc = mrb_define_class(mrb_, class_name, mrb_->object_class);
    MRB_SET_INSTANCE_TT(tc, MRB_TT_DATA);
    BindInstanceMethod(class_name, "initialize",
                       mrb_cptr_value(mrb_, (void*)new_func_ptr),
                       ClassBinder<Func>::ctor);
  }

  // Bind instance method.
  template <class Method>
  void bind_instance_method(const char* class_name, const char* method_name,
                            Method method_ptr) {
    mrb_value method_pptr_v = mrb_str_new(mrb_,
                                          reinterpret_cast<char*>(&method_ptr),
                                          sizeof(method_ptr));
    BindInstanceMethod(class_name, method_name,
                       method_pptr_v, ClassBinder<Method>::call);
  }

  // Bind static method.
  template <class Method>
  void bind_static_method(const char* class_name, const char* method_name,
                          Method method_ptr) {
    mrb_sym method_name_s = mrb_intern_cstr(mrb_, method_name);
    mrb_value env[] = {
      mrb_cptr_value(mrb_, (void*)method_ptr),  // 0: method pointer
      mrb_symbol_value(method_name_s),          // 1: method name
    };
    struct RProc* proc = mrb_proc_new_cfunc_with_env(mrb_, Binder<Method>::call, 2, env);
    struct RClass* klass = GetClass(class_name);
    mrb_define_class_method_raw(mrb_, klass, method_name_s, proc);
  }

private:
  void Initialize();

  // Returns mruby class under a module.
  struct RClass* GetClass(const char* class_name);

  // Utility for binding instance method.
  void BindInstanceMethod(const char* class_name, const char* method_name,
                          mrb_value original_func_v,
                          mrb_value (*binder_func)(mrb_state*, mrb_value));

  // Mimic mruby API.
  // TODO: Send pull request to the official mruby repository.
  void
  mrb_define_class_method_raw(mrb_state *mrb, struct RClass *c, mrb_sym mid, struct RProc *p);

  mrb_state* mrb_;
  RClass* mod_;
  int arena_index_;
};

}  // namespace mrubybind

#endif
