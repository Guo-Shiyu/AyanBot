#pragma once

#include "fwd.h"
#include "service.h"

#include <atomic>

namespace ayan
{
    // void some_fn() {}
    // BinaryFnPtr fn_ptr = reinterpret_cast<size_t>(some_fn);
    using BinaryFnPtr = size_t;

    namespace discards
    {
        constexpr void _0() {}

        template <typename Ret = void>
        constexpr auto _1 = [](auto &&_) -> Ret {
            return Ret(0);
        };

        template <typename Ret = void>
        constexpr auto _2 = [](auto &&_, auto &&__) -> Ret {
            return Ret(0);
        };
    }

    template <typename EventType>
    struct ServiceAdaptor
    {
        using OptTrigStage = std::function<bool(Bot &, EventType &)>;
        using OptActStage = std::function<void(Bot &, EventType &)>;

        OptTrigStage trig = discards::_2<bool>;
        OptActStage act = discards::_2<void>;
    };

    template <typename T, void (*OnInit)(), void (*OnDrop)()>
    struct _RefCounterGuard
    {
        static inline std::atomic_int objects_alive = 0;

        _RefCounterGuard()
        {
            ++objects_alive;
            if (objects_alive == 1)
                std::invoke(OnInit);
        }

        _RefCounterGuard(const _RefCounterGuard &)
        {
            ++objects_alive;
        }

    protected:
        ~_RefCounterGuard()
        {
            --objects_alive;
            if (objects_alive == 0)
                std::invoke(OnDrop);
        }
    };
}

#ifdef LUA_INTERFACE
#include "sol/sol.hpp"

namespace ayan
{
    [[unstable]]
    void regist_ayan_api(sol::state &lua)
    {
        // open all lua standrd libraries
        lua.open_libraries();

        // create global table "Ayan"
        auto ayan = lua["Ayan"].get_or_create<sol::table>();

        // // regist api
        // ayan.new_usertype<TextSeg>("TextSeg",
        //         "wtext", &TextSeg::text)    // todo
        //     .new_usertype<FaceSeg>("FaceSeg",
        //         "id", &FaceSeg::id)
        //     .new_usertype<AtSeg>("AtSeg"
        //         "qq", &FaceSeg::qq)
        //     .new_usertype<ImageSeg>("ImageSeg"
        //         "file", &ImageSeg::file,
        //         "url", &ImageSeg::url,
        //         "is_flash", &ImageSeg::is_flash)
        //     .new_usertype<RecordSeg>("RecordSeg"
        //         "file", &RecordSeg::file,
        //         "url", &RecordSeg::url,
        //         "is_magic", &RecordSeg::is_magic)
        //     .new_usertype<ReplySeg>("ReplySeg",
        //         "id", &ReplySeg::id)
        //     .new_usertype<Message>("Message",
        //         "dump", &Message::dump);    // todo

        // ayan.new_usertype<MsgBuilder>("MsgBuilder",
        //     sol::constructors<MsgBuilder(const std::string&)>,
        //     "text", sol::resolve<MsgBuilder&(const std::string&)>(&MsgBuilder::text),
        //     "face", &MsgBuilder::face,
        //     "build", &MsgBuilder::build);

        // // ayan.new_usertype<Bot>("Bot",
        // // 	"f", &bot::f,
        // // 	"g", &bot::g); // the usual
    }

    // template<typename CppType, typename LuaType = sol::table>
    // void serlize_to(sol::state& state, std::string_view key, CppType&& value)
    // {

    // }
}

#endif // LUA_INTERFACE

#ifdef PYTHON_INTERFACE

// could not compile without this macro on 'Windows' platform
#undef HAVE_SYS_TIME_H
#include "pybind11/embed.h"

namespace py = pybind11;
using namespace py::literals;

namespace ayan
{
    namespace buildin_service
    {
        class PythonInterface
            : public MetaService<PythonInterface>
        {
        public:
            std::string_view name() const override
            {
                return "Python Interface";
            }

            void on_load(Bot &)
            {
                ++ServiceRefCounter;
                if (ServiceRefCounter == 1)
                    _python_interpreter_init();
            }

            void on_unload(Bot &)
            {
                --ServiceRefCounter;
                if (ServiceRefCounter == 0)
                    _python_interpreter_drop();
            }

        public:
            static inline std::atomic_int ServiceRefCounter = 0;

            static void _python_interpreter_init()
            {
                Py_SetPythonHome(utf8_to_wstr(INTERPRETER_PATH).c_str());
                Py_Initialize();
            }

            static void _python_interpreter_drop()
            {
                Py_Finalize();
            }
        };
    }
}

#endif // PYTHON_INTERFACE