# $NetBSD: Makefile,v 1.3 2019/11/03 10:18:27 rillig Exp $

DISTNAME=	fixtures-3.0.0
PKGNAME=	${PYPKGPREFIX}-${DISTNAME}
CATEGORIES=	devel python
MASTER_SITES=	${MASTER_SITE_PYPI:=f/fixtures/}

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	https://pypi.python.org/pypi/fixtures/
COMMENT=	Fixtures, reusable state for writing clean tests and more
LICENSE=	apache-2.0 OR modified-bsd

DEPENDS+=	${PYPKGPREFIX}-six>=1.0.0:../../lang/py-six

# 9 test failures as of 3.0.0
# https://github.com/testing-cabal/fixtures/issues/35
# TEST_DEPENDS
# needs py-testtools for testing, but that depends on this
#BUILD_DEPENDS+=	${PYPKGPREFIX}-testtools-[0-9]*:../../devel/py-testtools
BUILD_DEPENDS+=		${PYPKGPREFIX}-mock-[0-9]*:../../devel/py-mock
BUILD_DEPENDS+=		${PYPKGPREFIX}-pbr-[0-9]*:../../devel/py-pbr

.include "../../lang/python/egg.mk"
.include "../../mk/bsd.pkg.mk"
