
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})

AUX_SOURCE_DIRECTORY(. DIR_SRCS)

IF(MSVC)
  INCLUDE_DIRECTORIES(${OPENSSL_ROOT_DIR}/include/)
  LINK_LIBRARIES(${OPENSSL_ROOT_DIR}/lib/libeay32.lib ${OPENSSL_ROOT_DIR}/lib/ssleay32.lib)
  LINK_LIBRARIES(ws2_32.lib advapi32.lib)
ELSE()
  LINK_LIBRARIES(ssl crypto)
  LINK_LIBRARIES(dl pthread)
ENDIF()

ADD_EXECUTABLE(test_crypt test_crypt.cpp ffile.cpp crypt.cpp)
ADD_EXECUTABLE(test_kvstore kvstore.cpp test_kvstore.cpp ffile.cpp)

ADD_EXECUTABLE(ysid_httpd ysid_httpd.cpp mongoose.c ffile.cpp crypt.cpp kvstore.cpp item.cpp option_parser.cpp
  crypt_store.cpp pair_store.cpp ysid_db.cpp ysid_httpd_server.cpp clipboard.cpp)

IF(MSVC)
ELSE()
  ADD_EXECUTABLE(ysid_from_pgman ysid_from_pgman.cpp ffile.cpp crypt.cpp item.cpp kvstore.cpp
    pair_store.cpp crypt_store.cpp ysid_db.cpp option_parser.cpp)
  ADD_EXECUTABLE(ysid_mock ysid_mock.cpp ffile.cpp crypt.cpp item.cpp kvstore.cpp
    pair_store.cpp crypt_store.cpp ysid_db.cpp option_parser.cpp)

  SET(QT_USE_QTOPENGL TRUE)
  FIND_PACKAGE(Qt4 REQUIRED)
  INCLUDE(${QT_USE_FILE})
  ADD_DEFINITIONS(${QT_DEFINITIONS})

  QT4_WRAP_UI(ysid_qt_UIS_H logindialog.ui editdialog.ui ysidmain.ui optionsdialog.ui)
  QT4_WRAP_CPP(ysid_qt_MOCS logindialog.hpp editdialog.hpp ysidmain.hpp optionsdialog.hpp ysid_db_model.hpp ysiddbview.hpp GlobalHot.hpp)
  QT4_ADD_RESOURCES(ysid_qt_RCC_SRCS ysid_qt.qrc)

  ADD_EXECUTABLE(ysid_qt ysid_qt.cpp
    logindialog.cpp editdialog.cpp optionsdialog.cpp ysidmain.cpp
    ysid_db_model.cpp ysiddbview.cpp
    ${ysid_qt_MOCS} ${ysid_qt_UIS_H} ${ysid_qt_RCC_SRCS}
    GlobalHot.cpp
    ffile.cpp crypt.cpp kvstore.cpp item.cpp option_parser.cpp crypt_store.cpp pair_store.cpp ysid_db.cpp
    ysid_httpd_server.cpp clipboard.cpp
    mongoose.c xdo.c)

  TARGET_LINK_LIBRARIES(ysid_qt ${QT_LIBRARIES} X11 Xtst Xinerama)

ENDIF()
