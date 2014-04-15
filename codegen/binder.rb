# This code is compiled by mrbc,
# and included into C code.

module MrubyBind
  def MrubyBind.define_function(mod, binder, func_name, func_ptr, nparam)
    if mod == Kernel
      # Not sure why defining class method for Kernel is not work with code below...
      Kernel.send(:define_method, func_name) do |*args|
        if args.size != nparam
          raise ArgumentError.new("'#{func_name}': wrong number of arguments (#{args.size} for #{nparam})")
        end
        MrubyBind::call_cfunc(binder, func_ptr, *args)
      end
    else
      mod.define_singleton_method(func_name) do |*args|
        if args.size != nparam
          raise ArgumentError.new("'#{func_name}': wrong number of arguments (#{args.size} for #{nparam})")
        end
        MrubyBind::call_cfunc(binder, func_ptr, *args)
      end
    end
  end

  def MrubyBind.bind_class(mod, binder, class_name, new_func_ptr, nparam)
    mod.const_get(class_name).class_eval do
      define_method(:initialize) do |*args|
        if args.size != nparam
          raise ArgumentError.new("'initialize': wrong number of arguments (#{args.size} for #{nparam})")
        end
        MrubyBind::call_ctorfunc(binder, self, new_func_ptr, *args)
      end
    end
  end

  def MrubyBind.bind_instance_method(mod, binder, class_name, method_name, method_ptr, nparam)
    mod.const_get(class_name).class_eval do
      define_method(method_name) do |*args|
        if args.size != nparam
          raise ArgumentError.new("'#{method_name}': wrong number of arguments (#{args.size} for #{nparam})")
        end
        MrubyBind::call_imethod(binder, self, method_ptr, *args)
      end
    end
  end

  def MrubyBind.bind_static_method(mod, binder, class_name, method_name, method_ptr, nparam)
    mod.const_get(class_name).class_eval do
      self.class.instance_eval do
        define_method(method_name) do |*args|
          if args.size != nparam
            raise ArgumentError.new("'#{method_name}': wrong number of arguments (#{args.size} for #{nparam})")
          end
          MrubyBind::call_cfunc(binder, method_ptr, *args)
        end
      end
    end
  end
end
