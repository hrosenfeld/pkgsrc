# $NetBSD: Makefile,v 1.4 2020/03/12 16:10:14 adam Exp $

DISTNAME=	paho-mqtt-1.5.0
PKGNAME=	${PYPKGPREFIX}-${DISTNAME}
CATEGORIES=	net python
MASTER_SITES=	${MASTER_SITE_PYPI:=p/paho-mqtt/}

MAINTAINER=	gdt@NetBSD.org
HOMEPAGE=	https://www.eclipse.org/paho/
COMMENT=	MQTT version 3.1.1 client class
LICENSE=	epl-v1.0

BUILD_DEPENDS+=	${PYPKGPREFIX}-test-runner-[0-9]*:../../devel/py-test-runner
# tests are not included
#TEST_DEPENDS+=	${PYPKGPREFIX}-lama>=7.6.6:../../devel/py-lama
#TEST_DEPENDS+=	${PYPKGPREFIX}-six-[0-9]*:../../lang/py-six
#TEST_DEPENDS+=	${PYPKGPREFIX}-test>=2.8.0:../../devel/py-test

USE_LANGUAGES=	# none

.include "../../lang/python/egg.mk"
.include "../../mk/bsd.pkg.mk"
