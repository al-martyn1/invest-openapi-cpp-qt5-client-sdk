@if "%TKF_IOA_QTROOT%" NEQ "" goto QTROOT_DEFINED
@rem set TKF_IOA_QTROOT=D:\Qt\Qt5.14.0\5.14.0
@rem Set to invalid version
@set TKF_IOA_QTROOT=D:\Qt\Qt5.35.0\5.35.0

:QTROOT_DEFINED

@if "%TKF_IOA_QTVER%" NEQ "" goto QTVER_DEFINED
@rem set TKF_IOA_QTVER=5.14.0
@rem Set to invalid version
@set TKF_IOA_QTVER=5.35.0

:QTVER_DEFINED


@set QTROOT=%TKF_IOA_QTROOT%
@set QTVER=%TKF_IOA_QTVER%

