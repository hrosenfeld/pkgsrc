# $NetBSD: Makefile,v 1.9 2020/05/09 20:16:59 rillig Exp $

DISTNAME=	ecdsa-0.15
PKGNAME=	${PYPKGPREFIX}-ecdsa-0.15
CATEGORIES=	security
MASTER_SITES=	${MASTER_SITE_PYPI:=e/ecdsa/}

MAINTAINER=	gls@NetBSD.org
HOMEPAGE=	https://github.com/warner/python-ecdsa/
COMMENT=	Easy-to-use implementation of ECDSA cryptography
LICENSE=	mit

DEPENDS+=	${PYPKGPREFIX}-six-[0-9]*:../../lang/py-six

USE_LANGUAGES=	# none

.include "../../lang/python/application.mk"
.include "../../lang/python/egg.mk"
.include "../../mk/bsd.pkg.mk"
